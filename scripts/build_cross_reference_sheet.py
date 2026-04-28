from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
from pathlib import Path

import xlrd


def normalize_text(value: str | None) -> str:
    if value is None:
        return ""
    return str(value).strip()


@dataclass
class CrossReferenceRow:
    lcsc_pn: str
    manufacturer: str
    manufacturer_pn: str
    description: str
    package: str
    quantity: int
    stock_status: str
    availability: str
    matched_status: str
    product_link: str
    source_file: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build a reusable LCSC-to-onshore cross-reference sheet from an LCSC quotation workbook."
    )
    parser.add_argument("input_xls", type=Path, help="Path to an LCSC export_project_*.xls quotation workbook.")
    parser.add_argument(
        "--output-csv",
        type=Path,
        help="Output CSV path. Defaults to the input workbook directory.",
    )
    return parser.parse_args()


def find_header_row(sheet: xlrd.sheet.Sheet) -> tuple[int, list[str]]:
    for row_index in range(sheet.nrows):
        row = [normalize_text(cell) for cell in sheet.row_values(row_index)]
        if "Matched status" in row and "Product Link" in row:
            return row_index, row
    raise ValueError("Could not find LCSC quotation header row.")


def safe_int(value: str) -> int:
    try:
        return int(float(value))
    except ValueError:
        return 0


def read_quote_rows(input_xls: Path) -> list[CrossReferenceRow]:
    workbook = xlrd.open_workbook(input_xls.as_posix())
    sheet = workbook.sheet_by_index(0)
    header_row_index, headers = find_header_row(sheet)
    rows: list[CrossReferenceRow] = []

    for row_index in range(header_row_index + 1, sheet.nrows):
        values = [normalize_text(cell) for cell in sheet.row_values(row_index)]
        if not any(values):
            continue
        row = dict(zip(headers, values))
        matched_status = normalize_text(row.get("Matched status", ""))
        manufacturer_pn = normalize_text(row.get("Mrf#", ""))
        lcsc_pn = normalize_text(row.get("LCSC#", ""))

        if manufacturer_pn.startswith("-("):
            continue
        if not any((lcsc_pn, manufacturer_pn, row.get("Description", ""))):
            continue

        rows.append(
            CrossReferenceRow(
                lcsc_pn=lcsc_pn,
                manufacturer=normalize_text(row.get("Mfr.", "")),
                manufacturer_pn=manufacturer_pn,
                description=normalize_text(row.get("Description", "")),
                package=normalize_text(row.get("Package", "")),
                quantity=safe_int(normalize_text(row.get("Quantity", "0"))),
                stock_status=normalize_text(row.get("Stock Status", "")),
                availability=normalize_text(row.get("Availability", "")),
                matched_status=matched_status,
                product_link=normalize_text(row.get("Product Link", "")),
                source_file=input_xls.name,
            )
        )

    return rows


def dedupe_rows(rows: list[CrossReferenceRow]) -> list[CrossReferenceRow]:
    deduped: dict[tuple[str, str, str], CrossReferenceRow] = {}
    for row in rows:
        key = (row.lcsc_pn, row.manufacturer, row.manufacturer_pn)
        existing = deduped.get(key)
        if existing is None or row.quantity > existing.quantity:
            deduped[key] = row
    return sorted(
        deduped.values(),
        key=lambda item: (
            item.manufacturer.lower(),
            item.manufacturer_pn.lower(),
            item.lcsc_pn.lower(),
        ),
    )


def write_cross_reference(output_csv: Path, rows: list[CrossReferenceRow]) -> None:
    output_csv.parent.mkdir(parents=True, exist_ok=True)
    with output_csv.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=[
                "LCSC_PN",
                "Manufacturer",
                "Manufacturer_PN",
                "Description",
                "Package",
                "Matched_Status",
                "Stock_Status",
                "Availability",
                "Example_Qty",
                "DigiKey_PN",
                "Mouser_PN",
                "Onshore_Preferred_PN",
                "Onshore_Preferred_Supplier",
                "Preferred_Status",
                "Notes",
                "Product_Link",
                "Source_File",
            ],
        )
        writer.writeheader()
        for row in rows:
            writer.writerow(
                {
                    "LCSC_PN": row.lcsc_pn,
                    "Manufacturer": row.manufacturer,
                    "Manufacturer_PN": row.manufacturer_pn,
                    "Description": row.description,
                    "Package": row.package,
                    "Matched_Status": row.matched_status,
                    "Stock_Status": row.stock_status,
                    "Availability": row.availability,
                    "Example_Qty": row.quantity,
                    "DigiKey_PN": "",
                    "Mouser_PN": "",
                    "Onshore_Preferred_PN": "",
                    "Onshore_Preferred_Supplier": "",
                    "Preferred_Status": "",
                    "Notes": "",
                    "Product_Link": row.product_link,
                    "Source_File": row.source_file,
                }
            )


def main() -> int:
    args = parse_args()
    input_xls = args.input_xls
    output_csv = args.output_csv or input_xls.with_name(f"{input_xls.stem}_Cross_Reference.csv")

    rows = read_quote_rows(input_xls)
    deduped_rows = dedupe_rows(rows)
    write_cross_reference(output_csv, deduped_rows)

    print(f"Input workbook: {input_xls}")
    print(f"Raw rows read: {len(rows)}")
    print(f"Cross-reference rows: {len(deduped_rows)}")
    print(f"Cross-reference CSV: {output_csv}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())