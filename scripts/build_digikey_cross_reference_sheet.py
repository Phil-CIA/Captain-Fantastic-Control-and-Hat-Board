from __future__ import annotations

import argparse
import csv
import re
from dataclasses import dataclass
from pathlib import Path

from jlc_to_digikey_bom import normalize_text, safe_stem


REQUIRED_COLUMNS = {
    "digikey_pn": ("DigiKey Part #", "Digi-Key Part Number"),
    "manufacturer_pn": ("Manufacturer Part Number", "Manufacturer Part #"),
    "description": ("Description",),
    "quantity": ("Quantity",),
    "unit_price": ("Unit Price",),
    "extended_price": ("Extended Price",),
}

PACKAGE_RE = re.compile(
    r"\b(0201|0402|0603|0805|1206|1210|1812|2010|2512|SOT-23-6|SOT-23|SOT-223|"
    r"SOIC-8(?:-\d+MIL)?|SOIC-14|SOIC-16|TQFN-16(?:-EP)?(?:\([^)]*\))?|QFN-16(?:-EP)?(?:\([^)]*\))?|"
    r"LFCSP-28(?:\([^)]*\))?|HTSSOP|TSSOP-\d+|DIP-\d+|SMD-\d+P|Through Hole(?:,P=\d+\.\d+mm)?)\b",
    re.IGNORECASE,
)

PASSIVE_VALUE_RE = re.compile(
    r"\b(\d+(?:\.\d+)?(?:pF|nF|uF|mF|F|ohm|k ohm|m ohm|kohm|mohm|Ω|kΩ|MΩ|V))\b",
    re.IGNORECASE,
)


@dataclass
class DigiKeySeedRow:
    manufacturer: str
    manufacturer_pn: str
    digikey_pn: str
    normalized_description: str
    package: str
    value_or_spec: str
    last_order_qty: int
    total_purchased_qty_ytd: int
    last_unit_price_usd: str
    last_extended_price_usd: str
    order_count: int
    customer_references: str
    preferred_onshore: str
    internal_pn: str
    status: str
    notes: str
    source_file: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build a Digi-Key-history-seeded cross-reference sheet from a standard Digi-Key order CSV export."
    )
    parser.add_argument("input_csv", type=Path, help="Path to a Digi-Key order CSV export.")
    parser.add_argument(
        "--output-csv",
        type=Path,
        help="Output CSV path. Defaults next to the input file.",
    )
    return parser.parse_args()


def find_column_map(fieldnames: list[str] | None) -> dict[str, str]:
    if fieldnames is None:
        raise ValueError("CSV does not contain a header row.")

    available = {normalize_text(name): name for name in fieldnames if name is not None}
    mapping: dict[str, str] = {}

    for target, candidates in REQUIRED_COLUMNS.items():
        source_name = next((available[candidate] for candidate in candidates if candidate in available), None)
        if source_name is None:
            joined = ", ".join(candidates)
            raise ValueError(f"Could not find required Digi-Key column: {joined}")
        mapping[target] = source_name

    mapping["customer_reference"] = available.get("Customer Reference", "")
    mapping["index"] = available.get("Index", available.get("\ufeffIndex", ""))
    return mapping


def parse_int(value: str) -> int:
    try:
        return int(normalize_text(value))
    except ValueError:
        return 0


def parse_currency(value: str) -> str:
    cleaned = re.sub(r"[^0-9.+-]", "", normalize_text(value))
    if not cleaned:
        return ""
    try:
        return f"{float(cleaned):.5f}".rstrip("0").rstrip(".")
    except ValueError:
        return cleaned


def normalize_description(description: str) -> str:
    description = normalize_text(description)
    description = re.sub(r"\s+", " ", description)
    return description


def extract_package(description: str) -> str:
    match = PACKAGE_RE.search(description)
    if not match:
        return ""
    return match.group(1).upper()


def extract_value_or_spec(description: str) -> str:
    match = PASSIVE_VALUE_RE.search(description)
    if not match:
        return ""
    return match.group(1).replace(" ", "")


def build_seed_rows(input_csv: Path) -> list[DigiKeySeedRow]:
    with input_csv.open("r", newline="", encoding="utf-8-sig") as handle:
        reader = csv.DictReader(handle)
        column_map = find_column_map(reader.fieldnames)
        deduped: dict[str, DigiKeySeedRow] = {}

        for row in reader:
            manufacturer_pn = normalize_text(row.get(column_map["manufacturer_pn"], ""))
            digikey_pn = normalize_text(row.get(column_map["digikey_pn"], ""))
            description = normalize_description(row.get(column_map["description"], ""))
            quantity = parse_int(row.get(column_map["quantity"], ""))
            unit_price = parse_currency(row.get(column_map["unit_price"], ""))
            extended_price = parse_currency(row.get(column_map["extended_price"], ""))
            customer_reference = normalize_text(row.get(column_map["customer_reference"], ""))
            line_index = normalize_text(row.get(column_map["index"], ""))

            if not any((manufacturer_pn, digikey_pn, description)):
                continue

            key = manufacturer_pn or digikey_pn or description
            status = "Seeded_From_DigiKey_History" if manufacturer_pn else "Missing_MPN"
            package = extract_package(description)
            value_or_spec = extract_value_or_spec(description)
            line_note = "Derived from Digi-Key history using the standard order CSV shape."
            if not manufacturer_pn:
                line_note = "Missing manufacturer part number in Digi-Key history; review before using as a stable key."

            existing = deduped.get(key)
            if existing is None:
                deduped[key] = DigiKeySeedRow(
                    manufacturer="",
                    manufacturer_pn=manufacturer_pn,
                    digikey_pn=digikey_pn,
                    normalized_description=description,
                    package=package,
                    value_or_spec=value_or_spec,
                    last_order_qty=quantity,
                    total_purchased_qty_ytd=quantity,
                    last_unit_price_usd=unit_price,
                    last_extended_price_usd=extended_price,
                    order_count=1,
                    customer_references=customer_reference,
                    preferred_onshore=digikey_pn,
                    internal_pn="",
                    status=status,
                    notes=line_note if not line_index else f"{line_note} Source line index: {line_index}.",
                    source_file=input_csv.name,
                )
                continue

            references = {part for part in existing.customer_references.split(" | ") if part}
            if customer_reference:
                references.add(customer_reference)

            existing.last_order_qty = quantity or existing.last_order_qty
            existing.total_purchased_qty_ytd += quantity
            existing.last_unit_price_usd = unit_price or existing.last_unit_price_usd
            existing.last_extended_price_usd = extended_price or existing.last_extended_price_usd
            existing.order_count += 1
            existing.customer_references = " | ".join(sorted(references))
            existing.preferred_onshore = existing.preferred_onshore or digikey_pn
            existing.package = existing.package or package
            existing.value_or_spec = existing.value_or_spec or value_or_spec
            if existing.status != status:
                existing.status = "Missing_MPN"
            existing.notes = (
                "Merged multiple Digi-Key history rows for the same stable key; latest row in file retained for last-price fields."
            )

    return sorted(
        deduped.values(),
        key=lambda item: (
            item.manufacturer_pn.lower(),
            item.digikey_pn.lower(),
            item.normalized_description.lower(),
        ),
    )


def write_cross_reference(output_csv: Path, rows: list[DigiKeySeedRow]) -> None:
    output_csv.parent.mkdir(parents=True, exist_ok=True)
    with output_csv.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=[
                "Manufacturer",
                "Manufacturer_PN",
                "DigiKey_PN",
                "Normalized_Description",
                "Package",
                "Value_or_Spec",
                "Last_Order_Qty",
                "Total_Purchased_Qty_YTD",
                "Last_Unit_Price_USD",
                "Last_Extended_Price_USD",
                "Order_Count",
                "Customer_References",
                "LCSC_PN",
                "Preferred_Onshore",
                "Preferred_JLC",
                "Internal_PN",
                "Status",
                "Notes",
                "Source_File",
            ],
        )
        writer.writeheader()
        for row in rows:
            writer.writerow(
                {
                    "Manufacturer": row.manufacturer,
                    "Manufacturer_PN": row.manufacturer_pn,
                    "DigiKey_PN": row.digikey_pn,
                    "Normalized_Description": row.normalized_description,
                    "Package": row.package,
                    "Value_or_Spec": row.value_or_spec,
                    "Last_Order_Qty": row.last_order_qty,
                    "Total_Purchased_Qty_YTD": row.total_purchased_qty_ytd,
                    "Last_Unit_Price_USD": row.last_unit_price_usd,
                    "Last_Extended_Price_USD": row.last_extended_price_usd,
                    "Order_Count": row.order_count,
                    "Customer_References": row.customer_references,
                    "LCSC_PN": "",
                    "Preferred_Onshore": row.preferred_onshore,
                    "Preferred_JLC": "",
                    "Internal_PN": row.internal_pn,
                    "Status": row.status,
                    "Notes": row.notes,
                    "Source_File": row.source_file,
                }
            )


def main() -> int:
    args = parse_args()
    input_csv = args.input_csv
    output_csv = args.output_csv or input_csv.with_name(f"{safe_stem(input_csv)}_DigiKey_Cross_Reference.csv")

    rows = build_seed_rows(input_csv)
    write_cross_reference(output_csv, rows)

    seeded_rows = sum(1 for row in rows if row.status == "Seeded_From_DigiKey_History")
    missing_mpn_rows = len(rows) - seeded_rows

    print(f"Input CSV: {input_csv}")
    print(f"Cross-reference CSV: {output_csv}")
    print(f"Rows written: {len(rows)}")
    print(f"Seeded from Digi-Key history: {seeded_rows}")
    print(f"Rows missing manufacturer MPN: {missing_mpn_rows}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())