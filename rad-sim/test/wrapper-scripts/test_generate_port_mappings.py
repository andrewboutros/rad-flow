import unittest
from unittest.mock import Mock, call
from scripts import generate_port_mappings as gpm
from scripts.verilog_parser import VerilogParameter, VerilogModule

mock_master_module = VerilogModule("mock_master_module", [
        VerilogParameter("input_w1", "input"), # Port 0: Input with width 1 (bool)
        VerilogParameter("input_w2", "input", "[1:0]"), # Port 1: Input with width 2
        VerilogParameter("output_w3", "output", "wire [5:3]"), # Port 2: Output with width 3
        VerilogParameter("inout_w2_reg", "inout", "reg [1:0]"), # Port 3: Inout reg with width 2
        VerilogParameter("output_w1_logic", "output", "logic"), # Port 4: Output logic with width 1 (bool)
        VerilogParameter("axis_mock_master_module_interface_tdata", "output", "logic [`DEF1:`DEF2]") # Port 5: Output AXI-S logic with unknown width
    ])

mock_slave_module = VerilogModule("mock_slave_module", [
        VerilogParameter("axis_mock_slave_module_interface_tdata", "input", "logic [`DEF1:`DEF2]") # Port 0: Input AXI-S logic with unknown width
    ])

mock_generate_module = VerilogModule("mock_generate_module", [
        VerilogParameter("axis_mock_generate_module_interface_tdata", "input", "logic [`DEF1:`DEF2]"), # Port 0: Input AXI-S logic with unknown width
        VerilogParameter("i64", "input", "[63:0]"), # Port 1: Input 64-bit bit vector
        VerilogParameter("o1", "output"), # Port 2: Output 1-bit boolean
        VerilogParameter("io32", "inout", "[31:0]"), # Port 3: Inout 32-bit bit-vector
        VerilogParameter("iox", "inout", "[`DEF1:`DEF2]") # Port 4: Inout with unknown width bit-vector
    ])
mock_generate_module_port_mappings = [
    call("module mock_generate_module\n"),
    call("axis slave axis_mock_generate_module_interface_tdata axis_mock_generate_module_interface.tdata\n"),
    call("input 64 i64 i64\n"),
    call("output 1 o1 o1\n"),
    call("inout 32 io32 io32\n"),
    call("inout ? iox iox\n")
]

class GeneratePortMappingsTest(unittest.TestCase):
    """
    GeneratePortMappingsTest class to test the functions in generate_port_mappings.py
    """

    def test_determine_port_width(self):
        """
        Tests the determine_port_width function for correct output
        """
        self.assertEqual(gpm.determine_port_width(mock_master_module.ports[0]), 1)
        self.assertEqual(gpm.determine_port_width(mock_master_module.ports[1]), 2)
        self.assertEqual(gpm.determine_port_width(mock_master_module.ports[2]), 3)
        self.assertEqual(gpm.determine_port_width(mock_master_module.ports[3]), 2)
        self.assertEqual(gpm.determine_port_width(mock_master_module.ports[4]), 1)
        self.assertEqual(gpm.determine_port_width(mock_master_module.ports[5]), "?")
    
    def test_is_axis_port(self):
        """
        Tests the is_axis_port function for correct output
        """
        self.assertFalse(gpm.is_axis_port(mock_master_module.ports[0], mock_master_module.name))
        self.assertTrue(gpm.is_axis_port(mock_master_module.ports[5], mock_master_module.name))

    def test_is_axis_role_master(self):
        """
        Tests the is_axis_role_master function for correct output
        """
        self.assertTrue(gpm.is_axis_role_master(mock_master_module))
        self.assertFalse(gpm.is_axis_role_master(mock_slave_module))

    def test_generate_port_mappings_for_module(self):
        """
        Tests the generate_port_mappings_for_module function for correct output
        """
        mock_file = Mock()
        mock_file.write = Mock()
        warnings = gpm.generate_port_mappings_for_module(mock_file, mock_generate_module)

        mock_file.write.assert_has_calls(mock_generate_module_port_mappings)
        self.assertTrue(warnings, "Warnings should be generated when there exists a value that cannot be inferred automatically.")

if __name__ == "__main__":
    unittest.main()