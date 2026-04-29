"""Tests for jlc_to_digikey_bom utility functions."""

from __future__ import annotations

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from jlc_to_digikey_bom import (
    append_note,
    build_commodity_spec,
    classify_footprint,
    exclude_from_bom,
    is_passive_candidate,
    is_probable_manufacturer_part,
    normalize_text,
    normalize_value,
    parse_cap_dielectric,
    parse_cap_voltage,
    parse_quantity,
    parse_resistor_tolerance,
    parse_resistor_wattage,
    safe_stem,
    split_references,
)


class TestNormalizeText:
    def test_strips_whitespace(self) -> None:
        assert normalize_text("  hello  ") == "hello"

    def test_none_returns_empty(self) -> None:
        assert normalize_text(None) == ""

    def test_empty_returns_empty(self) -> None:
        assert normalize_text("") == ""

    def test_converts_non_string(self) -> None:
        assert normalize_text(42) == "42"  # type: ignore[arg-type]


class TestSafeStem:
    def test_simple_name(self) -> None:
        assert safe_stem(Path("project_BOM.csv")) == "project_BOM"

    def test_spaces_replaced(self) -> None:
        assert safe_stem(Path("Project BOM.csv")) == "Project_BOM"

    def test_special_chars_replaced(self) -> None:
        assert safe_stem(Path("board (rev1).csv")) == "board_rev1"

    def test_empty_stem_fallback(self) -> None:
        assert safe_stem(Path("!!.csv")) == "bom"


class TestNormalizeValue:
    def test_omega_replaced(self) -> None:
        assert "ohm" in normalize_value("100Ω")

    def test_mu_replaced(self) -> None:
        assert "u" in normalize_value("10µF")

    def test_whitespace_collapsed(self) -> None:
        assert normalize_value("  10  k  ") == "10 k"


class TestAppendNote:
    def test_empty_base(self) -> None:
        assert append_note("", "extra") == "extra"

    def test_empty_extra(self) -> None:
        assert append_note("base", "") == "base"

    def test_both_present(self) -> None:
        result = append_note("base", "extra")
        assert result == "base extra"

    def test_duplicate_extra_not_added(self) -> None:
        assert append_note("base extra", "extra") == "base extra"


class TestSplitReferences:
    def test_simple_split(self) -> None:
        assert split_references("R1, R2, R3") == ["R1", "R2", "R3"]

    def test_empty(self) -> None:
        assert split_references("") == []

    def test_strips_quotes(self) -> None:
        assert split_references('"R1, R2"') == ["R1", "R2"]


class TestParseQuantity:
    def test_qty_field(self) -> None:
        assert parse_quantity({"Qty": "5"}) == 5

    def test_quantity_field(self) -> None:
        assert parse_quantity({"Quantity": "3"}) == 3

    def test_falls_back_to_references(self) -> None:
        assert parse_quantity({"Reference": "R1, R2, R3"}) == 3

    def test_invalid_qty_falls_back(self) -> None:
        row = {"Qty": "n/a", "Reference": "R1, R2"}
        assert parse_quantity(row) == 2


class TestExcludeFromBom:
    @pytest.mark.parametrize("value", ["yes", "Yes", "YES", "true", "1", "x", "X"])
    def test_excluded_values(self, value: str) -> None:
        assert exclude_from_bom({"Exclude from BOM": value}) is True

    def test_not_excluded(self) -> None:
        assert exclude_from_bom({"Exclude from BOM": "no"}) is False

    def test_missing_field(self) -> None:
        assert exclude_from_bom({}) is False


class TestClassifyFootprint:
    def test_resistor_0402(self) -> None:
        prefix, package = classify_footprint("Resistor_SMD:R_0402")
        assert prefix == "R"
        assert package == "0402"

    def test_cap_0805(self) -> None:
        prefix, package = classify_footprint("Capacitor_SMD:C_0805")
        assert prefix == "C"
        assert package == "0805"

    def test_unrecognized(self) -> None:
        prefix, package = classify_footprint("Package_SOT-23:SOT-23")
        assert prefix is None
        assert package is None


class TestIsPassiveCandidate:
    def test_resistor_footprint(self) -> None:
        assert is_passive_candidate("10k", "Resistor_SMD:R_0402") is True

    def test_cap_footprint(self) -> None:
        assert is_passive_candidate("100nF", "Capacitor_SMD:C_0603") is True

    def test_ic_not_passive(self) -> None:
        assert is_passive_candidate("TPS1H200A", "Package_SO:SOIC-8") is False


class TestIsProbableManufacturerPart:
    def test_valid_part(self) -> None:
        assert is_probable_manufacturer_part("RC0402FR-0710KL") is True

    def test_too_short(self) -> None:
        assert is_probable_manufacturer_part("R1") is False

    def test_generic_token(self) -> None:
        assert is_probable_manufacturer_part("~") is False

    def test_empty(self) -> None:
        assert is_probable_manufacturer_part("") is False


class TestParseResistorTolerance:
    def test_1_percent(self) -> None:
        assert parse_resistor_tolerance("RC0402FR-0710KL") == "1%"

    def test_5_percent(self) -> None:
        assert parse_resistor_tolerance("RC0402JR-0710KL") == "5%"

    def test_no_match(self) -> None:
        assert parse_resistor_tolerance("10K") == ""


class TestParseResistorWattage:
    def test_0402(self) -> None:
        assert parse_resistor_wattage("0402") == "62.5mW"

    def test_0805(self) -> None:
        assert parse_resistor_wattage("0805") == "125mW"

    def test_unknown_package(self) -> None:
        assert parse_resistor_wattage("2512") == ""

    def test_none(self) -> None:
        assert parse_resistor_wattage(None) == ""


class TestParseCapDielectric:
    def test_x7r(self) -> None:
        assert parse_cap_dielectric("100nF X7R 0402") == "X7R"

    def test_c0g(self) -> None:
        assert parse_cap_dielectric("C0G_100pF") == "C0G"

    def test_no_match(self) -> None:
        assert parse_cap_dielectric("10K") == ""


class TestParseCapVoltage:
    def test_explicit_voltage_in_value(self) -> None:
        assert parse_cap_voltage("100nF 50V", "") == "50V"

    def test_voltage_in_part(self) -> None:
        assert parse_cap_voltage("100nF", "GRM155R71H104KE14D") == "50V"

    def test_no_voltage(self) -> None:
        assert parse_cap_voltage("100nF", "UNKNOWN") == ""


class TestBuildCommoditySpec:
    def test_resistor_spec(self) -> None:
        spec = build_commodity_spec("10k", "Resistor_SMD:R_0402", "RC0402FR-0710KL")
        assert spec.startswith("RES")
        assert "0402" in spec

    def test_cap_spec(self) -> None:
        spec = build_commodity_spec(
            "100nF", "Capacitor_SMD:C_0603", "GRM188R71H104KA93D"
        )
        assert spec.startswith("CAP")
        assert "0603" in spec

    def test_unrecognized_returns_empty(self) -> None:
        assert (
            build_commodity_spec("TPS1H200A", "Package_SO:SOIC-8", "TPS1H200A-Q1") == ""
        )
