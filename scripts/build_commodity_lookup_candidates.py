from __future__ import annotations

import argparse
import csv
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path

from jlc_to_digikey_bom import (
    build_commodity_spec,
    exclude_from_bom,
    is_passive_candidate,
    normalize_text,
    parse_quantity,
    read_rows,
)


@dataclass
class CommodityAggregate:
    commodity_spec: str
    total_quantity: int = 0
    bom_count: int = 0
    example_value: str = ""
    example_footprint: str = ""
    example_manufacturer: str = ""
    example_manufacturer_pn: str = ""
    example_lcsc: str = ""
    source_boms: set[str] | None = None

    def __post_init__(self) -> None:
        if self.source_boms is None:
            self.source_boms = set()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Scan JLC BOM CSVs and build a normalized commodity lookup-candidates CSV."
    )
    parser.add_argument(
        "input_path",
        type=Path,
        help="A BOM CSV file or a directory tree containing JLC BOM CSV files.",
    )
    parser.add_argument(
        "--output-csv",
        type=Path,
        help="Output CSV path. Defaults to commodity_lookup_candidates.csv in the input directory.",
    )
    return parser.parse_args()


def find_csv_files(input_path: Path) -> list[Path]:
    if input_path.is_file():
        return [input_path]
    return sorted(path for path in input_path.rglob("*.csv") if path.is_file())


def aggregate_commodity_specs(csv_files: list[Path]) -> dict[str, CommodityAggregate]:
    aggregates: dict[str, CommodityAggregate] = {}

    for csv_path in csv_files:
        for row in read_rows(csv_path):
            if exclude_from_bom(row):
                continue

            value = normalize_text(row.get("Value", ""))
            footprint = normalize_text(row.get("Footprint", ""))
            manufacturer = normalize_text(row.get("Manufacturer", ""))
            manufacturer_pn = normalize_text(row.get("Part", ""))
            lcsc = normalize_text(row.get("LCSC", ""))

            if not is_passive_candidate(value, footprint):
                continue

            commodity_spec = build_commodity_spec(value, footprint, manufacturer_pn)
            if not commodity_spec:
                continue

            aggregate = aggregates.setdefault(commodity_spec, CommodityAggregate(commodity_spec=commodity_spec))
            aggregate.total_quantity += parse_quantity(row)
            aggregate.source_boms.add(str(csv_path))
            aggregate.bom_count = len(aggregate.source_boms)

            if not aggregate.example_value:
                aggregate.example_value = value
            if not aggregate.example_footprint:
                aggregate.example_footprint = footprint
            if not aggregate.example_manufacturer:
                aggregate.example_manufacturer = manufacturer
            if not aggregate.example_manufacturer_pn:
                aggregate.example_manufacturer_pn = manufacturer_pn
            if not aggregate.example_lcsc:
                aggregate.example_lcsc = lcsc

    return aggregates


def write_candidates(output_csv: Path, aggregates: dict[str, CommodityAggregate]) -> None:
    output_csv.parent.mkdir(parents=True, exist_ok=True)
    rows = sorted(aggregates.values(), key=lambda item: (-item.bom_count, -item.total_quantity, item.commodity_spec))

    with output_csv.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=[
                "Commodity_Spec",
                "DigiKey_PN",
                "Description",
                "Notes",
                "Seen_In_BOMs",
                "Total_Qty",
                "Example_Value",
                "Example_Footprint",
                "Example_Manufacturer",
                "Example_Manufacturer_PN",
                "Example_LCSC",
            ],
        )
        writer.writeheader()
        for item in rows:
            writer.writerow(
                {
                    "Commodity_Spec": item.commodity_spec,
                    "DigiKey_PN": "",
                    "Description": item.commodity_spec,
                    "Notes": "Fill with preferred Digi-Key stock code once validated.",
                    "Seen_In_BOMs": item.bom_count,
                    "Total_Qty": item.total_quantity,
                    "Example_Value": item.example_value,
                    "Example_Footprint": item.example_footprint,
                    "Example_Manufacturer": item.example_manufacturer,
                    "Example_Manufacturer_PN": item.example_manufacturer_pn,
                    "Example_LCSC": item.example_lcsc,
                }
            )


def main() -> int:
    args = parse_args()
    csv_files = find_csv_files(args.input_path)
    if not csv_files:
        raise SystemExit(f"No CSV files found under {args.input_path}")

    default_output = args.input_path if args.input_path.is_dir() else args.input_path.parent
    output_csv = args.output_csv or (default_output / "commodity_lookup_candidates.csv")

    aggregates = aggregate_commodity_specs(csv_files)
    write_candidates(output_csv, aggregates)

    print(f"Scanned CSV files: {len(csv_files)}")
    print(f"Commodity specs found: {len(aggregates)}")
    print(f"Lookup candidates CSV: {output_csv}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())