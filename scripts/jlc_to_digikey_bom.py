from __future__ import annotations

import argparse
import csv
import re
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import xlrd


PASSIVE_FOOTPRINT_RE = re.compile(r"(?:^|:)([RC])_(\d{4})$", re.IGNORECASE)
REFERENCE_SPLIT_RE = re.compile(r"\s*,\s*")


@dataclass(frozen=True)
class LookupEntry:
    commodity_spec: str
    digikey_pn: str
    description: str
    notes: str


@dataclass(frozen=True)
class CrossReferenceEntry:
    lcsc_pn: str
    manufacturer: str
    manufacturer_pn: str
    digikey_pn: str
    onshore_preferred_pn: str
    onshore_preferred_supplier: str
    preferred_status: str
    notes: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert a JLC-style BOM CSV or LCSC quotation XLS into Digi-Key candidate and upload files."
    )
    parser.add_argument("input_csv", type=Path, help="Path to the JLC BOM CSV or LCSC quotation XLS file.")
    parser.add_argument(
        "--output-dir",
        type=Path,
        help="Directory for generated files. Defaults to the input CSV directory.",
    )
    parser.add_argument(
        "--lookup-csv",
        type=Path,
        help=(
            "Optional CSV mapping commodity specs or manufacturer part numbers to Digi-Key part numbers. "
            "Expected columns: Commodity_Spec, DigiKey_PN, Description, Notes"
        ),
    )
    parser.add_argument(
        "--cross-reference-csv",
        type=Path,
        help=(
            "Optional reusable cross-reference CSV with columns such as LCSC_PN, Manufacturer_PN, "
            "DigiKey_PN, Onshore_Preferred_PN, and Onshore_Preferred_Supplier."
        ),
    )
    parser.add_argument(
        "--master-cross-reference-csv",
        type=Path,
        help=(
            "Optional master reusable cross-reference CSV. If both this and --cross-reference-csv are provided, "
            "the project cross-reference sheet is checked first and the master sheet is used as fallback."
        ),
    )
    return parser.parse_args()


def load_lookup_table(path: Path | None) -> dict[str, LookupEntry]:
    if path is None:
        return {}

    lookup: dict[str, LookupEntry] = {}
    with path.open("r", newline="", encoding="utf-8-sig") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            commodity_spec = normalize_text(row.get("Commodity_Spec", ""))
            if not commodity_spec:
                continue
            lookup[commodity_spec] = LookupEntry(
                commodity_spec=commodity_spec,
                digikey_pn=normalize_text(row.get("DigiKey_PN", "")),
                description=normalize_text(row.get("Description", "")),
                notes=normalize_text(row.get("Notes", "")),
            )
    return lookup


def load_cross_reference_table(
    path: Path | None,
) -> tuple[dict[str, CrossReferenceEntry], dict[str, CrossReferenceEntry]]:
    if path is None:
        return {}, {}

    by_lcsc: dict[str, CrossReferenceEntry] = {}
    by_manufacturer_pn: dict[str, CrossReferenceEntry] = {}

    with path.open("r", newline="", encoding="utf-8-sig") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            entry = CrossReferenceEntry(
                lcsc_pn=normalize_text(row.get("LCSC_PN", "")),
                manufacturer=normalize_text(row.get("Manufacturer", "")),
                manufacturer_pn=normalize_text(row.get("Manufacturer_PN", "")),
                digikey_pn=normalize_text(row.get("DigiKey_PN", "")),
                onshore_preferred_pn=normalize_text(row.get("Onshore_Preferred_PN", "")),
                onshore_preferred_supplier=normalize_text(row.get("Onshore_Preferred_Supplier", "")),
                preferred_status=normalize_text(row.get("Preferred_Status", "")),
                notes=normalize_text(row.get("Notes", "")),
            )
            if entry.lcsc_pn:
                by_lcsc.setdefault(entry.lcsc_pn, entry)
            if entry.manufacturer_pn:
                by_manufacturer_pn.setdefault(entry.manufacturer_pn, entry)

    return by_lcsc, by_manufacturer_pn


def normalize_text(value: str | None) -> str:
    if value is None:
        return ""
    return str(value).strip()


def append_note(base_note: str, extra_note: str) -> str:
    base_note = normalize_text(base_note)
    extra_note = normalize_text(extra_note)
    if not extra_note:
        return base_note
    if not base_note:
        return extra_note
    if extra_note in base_note:
        return base_note
    return f"{base_note} {extra_note}"


def safe_stem(path: Path) -> str:
    return re.sub(r"[^A-Za-z0-9._-]+", "_", path.stem).strip("_") or "bom"


def split_references(reference_field: str) -> list[str]:
    cleaned = normalize_text(reference_field).strip('"')
    if not cleaned:
        return []
    return [part for part in REFERENCE_SPLIT_RE.split(cleaned) if part]


def parse_quantity(row: dict[str, str]) -> int:
    raw_qty = normalize_text(row.get("Qty", "") or row.get("Quantity", ""))
    try:
        return int(raw_qty)
    except ValueError:
        return max(1, len(split_references(row.get("Reference", ""))))


def exclude_from_bom(row: dict[str, str]) -> bool:
    raw_value = normalize_text(row.get("Exclude from BOM", ""))
    return raw_value.lower() in {"yes", "true", "1", "x"}


def is_lcsc_quote_row_skippable(row: dict[str, str]) -> bool:
    matched_status = normalize_text(row.get("Matched status", ""))
    manufacturer_part = normalize_text(row.get("Part", ""))
    description = normalize_text(row.get("Description", ""))
    if matched_status.lower() == "exact matches":
        return False
    if manufacturer_part.startswith("-("):
        return True
    return not any((manufacturer_part, description, row.get("LCSC", "")))


def looks_generic_part_number(part_number: str) -> bool:
    lowered = part_number.lower()
    generic_tokens = {
        "conn_01x10_pin",
        "connector_esp32_1x15",
        "mountinghole",
        "spdt",
        "~",
    }
    return lowered in generic_tokens


def classify_footprint(footprint: str) -> tuple[str | None, str | None]:
    match = PASSIVE_FOOTPRINT_RE.search(normalize_text(footprint))
    if not match:
        return None, None
    return match.group(1).upper(), match.group(2)


def build_commodity_spec(value: str, footprint: str, manufacturer_part: str) -> str:
    prefix, package = classify_footprint(footprint)
    normalized_value = normalize_value(value)

    if prefix == "R":
        tolerance = parse_resistor_tolerance(manufacturer_part)
        wattage = parse_resistor_wattage(package)
        tokens = ["RES", normalized_value]
        if tolerance:
            tokens.append(tolerance)
        if package:
            tokens.append(package)
        if wattage:
            tokens.append(f">={wattage}")
        return " ".join(tokens)

    if prefix == "C":
        dielectric = parse_cap_dielectric(manufacturer_part)
        voltage = parse_cap_voltage(value, manufacturer_part)
        tokens = ["CAP", normalized_value]
        if dielectric:
            tokens.append(dielectric)
        if voltage:
            tokens.append(voltage)
        if package:
            tokens.append(package)
        return " ".join(tokens)

    return ""


def normalize_value(value: str) -> str:
    value = normalize_text(value)
    replacements = {
        "Ω": "ohm",
        "µ": "u",
        "μ": "u",
    }
    for old, new in replacements.items():
        value = value.replace(old, new)
    value = value.replace(",", " ")
    return re.sub(r"\s+", " ", value).strip()


def parse_resistor_tolerance(manufacturer_part: str) -> str:
    match = re.search(r"([FJ])", manufacturer_part)
    if not match:
        return ""
    return {"F": "1%", "J": "5%"}.get(match.group(1), "")


def parse_resistor_wattage(package: str | None) -> str:
    wattage_by_package = {
        "0402": "62.5mW",
        "0603": "100mW",
        "0805": "125mW",
        "1206": "250mW",
    }
    if not package:
        return ""
    return wattage_by_package.get(package, "")


def parse_cap_dielectric(manufacturer_part: str) -> str:
    dielectric_tokens = ("X7R", "X5R", "C0G", "NP0", "Y5V")
    for token in dielectric_tokens:
        if token in manufacturer_part.upper():
            return token
    return ""


def parse_cap_voltage(value: str, manufacturer_part: str) -> str:
    combined = f"{value} {manufacturer_part}"
    match = re.search(r"(\d+(?:\.\d+)?)\s*V", combined, re.IGNORECASE)
    if match:
        return f"{match.group(1)}V"

    voltage_by_series_token = {
        "BB": "50V",
        "1H": "50V",
        "1E": "25V",
        "1A": "10V",
    }
    upper = manufacturer_part.upper()
    for token, voltage in voltage_by_series_token.items():
        if token in upper:
            return voltage
    return ""


def is_passive_candidate(value: str, footprint: str) -> bool:
    prefix, _ = classify_footprint(footprint)
    if prefix in {"R", "C"}:
        return True
    return bool(re.search(r"(?:ohm|uf|nf|pf|μf|µf|f)$", normalize_value(value), re.IGNORECASE))


def is_probable_manufacturer_part(part_number: str) -> bool:
    part_number = normalize_text(part_number)
    if not part_number or looks_generic_part_number(part_number):
        return False
    if len(part_number) < 4:
        return False
    has_letter = any(ch.isalpha() for ch in part_number)
    has_digit = any(ch.isdigit() for ch in part_number)
    return has_letter and has_digit


def build_description(row: dict[str, str], commodity_spec: str) -> str:
    value = normalize_text(row.get("Value", ""))
    manufacturer = normalize_text(row.get("Manufacturer", ""))
    part = normalize_text(row.get("Part", ""))
    if commodity_spec:
        return commodity_spec
    if manufacturer and part:
        return f"{manufacturer} {part}".strip()
    if part:
        return part
    return value


def is_lcsc_quote_source(row: dict[str, str]) -> bool:
    return normalize_text(row.get("SourceKind", "")).upper() == "LCSC_QUOTE"


def safe_part_number_for_upload(
    manufacturer: str,
    manufacturer_part: str,
    digikey_pn: str,
    status: str,
) -> tuple[str, str]:
    if digikey_pn:
        return digikey_pn, "DigiKey_PN"
    if status == "Auto" and manufacturer and manufacturer_part:
        return manufacturer_part, "Manufacturer_PN"
    return "", ""


def find_cross_reference_entry(
    lcsc: str,
    manufacturer_part: str,
    by_lcsc: dict[str, CrossReferenceEntry],
    by_manufacturer_pn: dict[str, CrossReferenceEntry],
) -> tuple[CrossReferenceEntry | None, str]:
    if lcsc and lcsc in by_lcsc:
        return by_lcsc[lcsc], "LCSC_PN"
    if manufacturer_part and manufacturer_part in by_manufacturer_pn:
        return by_manufacturer_pn[manufacturer_part], "Manufacturer_PN"
    return None, ""


def find_preferred_cross_reference_entry(
    lcsc: str,
    manufacturer_part: str,
    project_by_lcsc: dict[str, CrossReferenceEntry],
    project_by_manufacturer_pn: dict[str, CrossReferenceEntry],
    master_by_lcsc: dict[str, CrossReferenceEntry],
    master_by_manufacturer_pn: dict[str, CrossReferenceEntry],
) -> tuple[CrossReferenceEntry | None, str]:
    project_entry, project_match = find_cross_reference_entry(
        lcsc,
        manufacturer_part,
        project_by_lcsc,
        project_by_manufacturer_pn,
    )
    if project_entry is not None:
        return project_entry, f"Project_{project_match}"

    master_entry, master_match = find_cross_reference_entry(
        lcsc,
        manufacturer_part,
        master_by_lcsc,
        master_by_manufacturer_pn,
    )
    if master_entry is not None:
        return master_entry, f"Master_{master_match}"

    return None, ""


def preferred_digikey_pn(entry: CrossReferenceEntry | None) -> str:
    if entry is None:
        return ""
    if entry.digikey_pn:
        return entry.digikey_pn

    supplier = entry.onshore_preferred_supplier.lower()
    if entry.onshore_preferred_pn and supplier in {"", "digikey", "digi-key", "digi key"}:
        return entry.onshore_preferred_pn
    return ""


def digikey_candidate_row(
    row: dict[str, str],
    lookup: dict[str, LookupEntry],
    project_cross_reference_by_lcsc: dict[str, CrossReferenceEntry],
    project_cross_reference_by_manufacturer_pn: dict[str, CrossReferenceEntry],
    master_cross_reference_by_lcsc: dict[str, CrossReferenceEntry],
    master_cross_reference_by_manufacturer_pn: dict[str, CrossReferenceEntry],
) -> tuple[dict[str, str] | None, dict[str, str] | None, str]:
    if exclude_from_bom(row):
        return None, None, "skipped"
    if is_lcsc_quote_source(row) and is_lcsc_quote_row_skippable(row):
        return None, None, "skipped"

    references = ", ".join(split_references(row.get("Reference", "")))
    qty = str(parse_quantity(row))
    value = normalize_text(row.get("Value", ""))
    footprint = normalize_text(row.get("Footprint", ""))
    lcsc = normalize_text(row.get("LCSC", ""))
    manufacturer = normalize_text(row.get("Manufacturer", ""))
    manufacturer_part = normalize_text(row.get("Part", ""))
    matched_status = normalize_text(row.get("Matched status", ""))
    cross_reference_entry, cross_reference_match = find_preferred_cross_reference_entry(
        lcsc,
        manufacturer_part,
        project_cross_reference_by_lcsc,
        project_cross_reference_by_manufacturer_pn,
        master_cross_reference_by_lcsc,
        master_cross_reference_by_manufacturer_pn,
    )

    commodity_spec = ""
    digikey_pn = ""
    status = "Manual_Review"
    notes = ""

    if is_lcsc_quote_source(row) and matched_status.lower() == "exact matches" and manufacturer and manufacturer_part:
        if is_passive_candidate(value, footprint):
            commodity_spec = build_commodity_spec(value, footprint, manufacturer_part)
        status = "Auto"
        notes = "Exact LCSC matched manufacturer part number available from quotation export."
    elif is_passive_candidate(value, footprint):
        commodity_spec = build_commodity_spec(value, footprint, manufacturer_part)
        lookup_entry = lookup.get(commodity_spec)
        if lookup_entry:
            digikey_pn = lookup_entry.digikey_pn
            status = "Lookup"
            notes = lookup_entry.notes or "Matched using commodity lookup table."
        else:
            status = "Lookup"
            notes = "Commodity passive needs Digi-Key mapping or preferred stock entry."
    elif manufacturer and is_probable_manufacturer_part(manufacturer_part):
        status = "Auto"
        notes = "Manufacturer and manufacturer part number available from source BOM."
    else:
        notes = "Missing reliable manufacturer part number or requires fit review."

    if cross_reference_entry is not None:
        manufacturer = manufacturer or cross_reference_entry.manufacturer
        manufacturer_part = manufacturer_part or cross_reference_entry.manufacturer_pn
        cross_reference_pn = preferred_digikey_pn(cross_reference_entry)
        if cross_reference_pn:
            digikey_pn = digikey_pn or cross_reference_pn
            status = "Auto"
        notes = append_note(
            notes,
            cross_reference_entry.notes or f"Cross-reference match via {cross_reference_match}.",
        )

    output_row = {
        "Reference": references,
        "Qty": qty,
        "Value": value,
        "Footprint": footprint,
        "Manufacturer": manufacturer,
        "Manufacturer_PN": manufacturer_part,
        "DigiKey_PN": digikey_pn,
        "Description": build_description(row, commodity_spec),
        "Commodity_Spec": commodity_spec,
        "Source_LCSC": lcsc,
        "Preferred_Onshore_PN": cross_reference_entry.onshore_preferred_pn if cross_reference_entry else "",
        "Preferred_Onshore_Supplier": cross_reference_entry.onshore_preferred_supplier if cross_reference_entry else "",
        "Cross_Reference_Match": cross_reference_match,
        "Status": status,
        "Notes": notes,
    }

    unresolved_row = None
    if status != "Auto":
        unresolved_row = {
            "Reference": references,
            "Qty": qty,
            "Value": value,
            "Footprint": footprint,
            "Manufacturer": manufacturer,
            "Manufacturer_PN": manufacturer_part,
            "Commodity_Spec": commodity_spec,
            "Source_LCSC": lcsc,
            "Preferred_Onshore_PN": cross_reference_entry.onshore_preferred_pn if cross_reference_entry else "",
            "Preferred_Onshore_Supplier": cross_reference_entry.onshore_preferred_supplier if cross_reference_entry else "",
            "Cross_Reference_Match": cross_reference_match,
            "Status": status,
            "Notes": notes,
        }

    return output_row, unresolved_row, status


def read_rows(input_csv: Path) -> Iterable[dict[str, str]]:
    if input_csv.suffix.lower() == ".xls":
        yield from read_lcsc_quote_rows(input_csv)
        return

    with input_csv.open("r", newline="", encoding="utf-8-sig") as handle:
        yield from csv.DictReader(handle)


def read_lcsc_quote_rows(input_xls: Path) -> Iterable[dict[str, str]]:
    workbook = xlrd.open_workbook(input_xls.as_posix())
    sheet = workbook.sheet_by_index(0)
    header_row_index = None

    for row_index in range(sheet.nrows):
        row = [normalize_text(str(value)) for value in sheet.row_values(row_index)]
        if "Matched status" in row and "Product Link" in row:
            header_row_index = row_index
            headers = row
            break

    if header_row_index is None:
        raise ValueError(f"Could not find LCSC quotation header row in {input_xls}")

    for row_index in range(header_row_index + 1, sheet.nrows):
        values = [normalize_text(str(value)) for value in sheet.row_values(row_index)]
        if not any(values):
            continue
        row = dict(zip(headers, values))
        yield {
            "Reference": normalize_text(row.get("Customer Part Number", "")),
            "Qty": normalize_text(row.get("Quantity", "")),
            "Value": normalize_text(row.get("Value", "") or row.get("Description", "")),
            "Footprint": normalize_text(row.get("Package", "")),
            "LCSC": normalize_text(row.get("LCSC#", "") or row.get("LCSC#", "")),
            "Manufacturer": normalize_text(row.get("Mfr.", "")),
            "Part": normalize_text(row.get("Mrf#", "")),
            "Matched status": normalize_text(row.get("Matched status", "")),
            "Description": normalize_text(row.get("Description", "")),
            "Exclude from BOM": "",
            "SourceKind": "LCSC_QUOTE",
        }


def write_csv(path: Path, fieldnames: list[str], rows: Iterable[dict[str, str]]) -> None:
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def main() -> int:
    args = parse_args()
    input_csv = args.input_csv
    output_dir = args.output_dir or input_csv.parent
    output_dir.mkdir(parents=True, exist_ok=True)

    lookup = load_lookup_table(args.lookup_csv)
    project_cross_reference_by_lcsc, project_cross_reference_by_manufacturer_pn = load_cross_reference_table(
        args.cross_reference_csv
    )
    master_cross_reference_by_lcsc, master_cross_reference_by_manufacturer_pn = load_cross_reference_table(
        args.master_cross_reference_csv
    )
    converted_rows: list[dict[str, str]] = []
    unresolved_rows: list[dict[str, str]] = []
    upload_rows: list[dict[str, str]] = []
    simple_upload_rows: list[dict[str, str]] = []
    status_counts: Counter[str] = Counter()

    for row in read_rows(input_csv):
        converted_row, unresolved_row, status = digikey_candidate_row(
            row,
            lookup,
            project_cross_reference_by_lcsc,
            project_cross_reference_by_manufacturer_pn,
            master_cross_reference_by_lcsc,
            master_cross_reference_by_manufacturer_pn,
        )
        status_counts[status] += 1
        if converted_row is not None:
            converted_rows.append(converted_row)
            requested_part_number, part_number_source = safe_part_number_for_upload(
                converted_row["Manufacturer"],
                converted_row["Manufacturer_PN"],
                converted_row["DigiKey_PN"],
                converted_row["Status"],
            )
            if requested_part_number:
                upload_rows.append(
                    {
                        "Requested Part Number": requested_part_number,
                        "Quantity": converted_row["Qty"],
                        "Reference Designator": converted_row["Reference"],
                        "Customer Reference": converted_row["Value"],
                        "Manufacturer Name": converted_row["Manufacturer"],
                        "Manufacturer Part Number": converted_row["Manufacturer_PN"],
                        "DigiKey Part Number": converted_row["DigiKey_PN"],
                        "Part Number Source": part_number_source,
                        "Description": converted_row["Description"],
                        "Source LCSC": converted_row["Source_LCSC"],
                    }
                )
                simple_upload_rows.append(
                    {
                        "Manufacturer Part Number": converted_row["Manufacturer_PN"] or requested_part_number,
                        "Quantity": converted_row["Qty"],
                        "Manufacturer Name": converted_row["Manufacturer"],
                    }
                )
        if unresolved_row is not None:
            unresolved_rows.append(unresolved_row)

    stem = safe_stem(input_csv)
    digikey_path = output_dir / f"{stem}_DigiKey_Candidate_BOM.csv"
    digikey_upload_path = output_dir / f"{stem}_DigiKey_Upload.csv"
    digikey_simple_upload_path = output_dir / f"{stem}_DigiKey_Simple_Upload.csv"
    unresolved_path = output_dir / f"{stem}_Unresolved.csv"

    write_csv(
        digikey_path,
        [
            "Reference",
            "Qty",
            "Value",
            "Footprint",
            "Manufacturer",
            "Manufacturer_PN",
            "DigiKey_PN",
            "Description",
            "Commodity_Spec",
            "Source_LCSC",
            "Preferred_Onshore_PN",
            "Preferred_Onshore_Supplier",
            "Cross_Reference_Match",
            "Status",
            "Notes",
        ],
        converted_rows,
    )
    write_csv(
        digikey_upload_path,
        [
            "Requested Part Number",
            "Quantity",
            "Reference Designator",
            "Customer Reference",
            "Manufacturer Name",
            "Manufacturer Part Number",
            "DigiKey Part Number",
            "Part Number Source",
            "Description",
            "Source LCSC",
        ],
        upload_rows,
    )
    write_csv(
        digikey_simple_upload_path,
        [
            "Manufacturer Part Number",
            "Quantity",
            "Manufacturer Name",
        ],
        simple_upload_rows,
    )
    write_csv(
        unresolved_path,
        [
            "Reference",
            "Qty",
            "Value",
            "Footprint",
            "Manufacturer",
            "Manufacturer_PN",
            "Commodity_Spec",
            "Source_LCSC",
            "Preferred_Onshore_PN",
            "Preferred_Onshore_Supplier",
            "Cross_Reference_Match",
            "Status",
            "Notes",
        ],
        unresolved_rows,
    )

    print(f"Input BOM: {input_csv}")
    print(f"Digi-Key candidate BOM: {digikey_path}")
    print(f"Digi-Key upload BOM: {digikey_upload_path}")
    print(f"Digi-Key simple upload BOM: {digikey_simple_upload_path}")
    print(f"Unresolved items report: {unresolved_path}")
    print("Status summary:")
    for label in ("Auto", "Lookup", "Manual_Review", "skipped"):
        print(f"  {label}: {status_counts.get(label, 0)}")
    print(f"  Upload rows: {len(upload_rows)}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())