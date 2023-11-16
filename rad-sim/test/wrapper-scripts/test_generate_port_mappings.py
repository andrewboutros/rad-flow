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
        VerilogParameter("axis_mock_master_module_tdata", "output", "logic [`DEF1:`DEF2]"), # Port 5: Output AXI-S logic with unknown width
        VerilogParameter("axis_mock_master_module_unknown_tunknown", "output") # Port 6: (unknown) Output AXI-S logic with width 1
    ])

mock_mixed_module = VerilogModule("mock_mixed_module", [
        VerilogParameter("axis_mock_master_module_tstrb", "output"), # Port 0: Output AXI-S logic with width 1
        VerilogParameter("axis_mock_slave_module_tid", "input", "logic [3:0]"), # Port 1: Input AXI-S logic with width 4
        VerilogParameter("axis_mock_unknown_module_tunknown", "input", "logic [511:0]") # Port 2: Input unknown AXI-S logic with width 512
    ])

mock_mixed_module_axis_roles = {
    "mock_master_module": "master",
    "mock_slave_module": "slave"
}

mock_generate_module = VerilogModule("mock_generate_module", [
        VerilogParameter("axis_mock_generate_module_0_tdata", "input", "logic [`DEF1:`DEF2]"), # Port 0: Input AXI-S logic with unknown width
        VerilogParameter("axis_mock_generate_module_1_tdata", "output", "logic [`DEF1:`DEF2]"), # Port 1: Output AXI-S logic with unknown width
        VerilogParameter("axis_mock_generate_module_2_tunknown", "output", "logic [`DEF1:`DEF2]"), # Port 2: Output unknown AXI-S logic with unknown width
        VerilogParameter("i64", "input", "[63:0]"), # Port 3: Input 64-bit bit vector
        VerilogParameter("o1", "output"), # Port 4: Output 1-bit boolean
        VerilogParameter("io32", "inout", "[31:0]"), # Port 5: Inout 32-bit bit-vector
        VerilogParameter("iox", "inout", "[`DEF1:`DEF2]") # Port 6: Inout with unknown width bit-vector
    ])

mock_generate_module_port_mappings = [
    call("module mock_generate_module\n"),
    call("axis slave axis_mock_generate_module_0_tdata axis_mock_generate_module_0 tdata\n"),
    call("axis master axis_mock_generate_module_1_tdata axis_mock_generate_module_1 tdata\n"),
    call("output ? axis_mock_generate_module_2_tunknown axis_mock_generate_module_2_tunknown\n"),
    call("input 64 i64 i64\n"),
    call("output 1 o1 o1\n"),
    call("inout 32 io32 io32\n"),
    call("inout ? iox iox\n")
]

mock_generate_module_axis_roles = {
    "mock_generate_module_0": "slave",
    "mock_generate_module_1": "master",
}

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
        self.assertEqual(gpm.determine_port_width(mock_master_module.ports[6]), 1)
    
    def test_is_axis_port(self):
        """
        Tests the is_axis_port function for correct output
        """
        self.assertFalse(gpm.is_axis_port(mock_master_module.ports[0]))
        self.assertTrue(gpm.is_axis_port(mock_master_module.ports[5]))

    def test_is_axis_role_found(self):
        """
        Tests the is_axis_port function for correct output
        """
        self.assertTrue(gpm.is_axis_role_found(mock_mixed_module_axis_roles, mock_mixed_module.ports[0]))
        self.assertFalse(gpm.is_axis_role_found(mock_mixed_module_axis_roles, mock_mixed_module.ports[2]))

    def test_determine_axis_roles(self):
        """
        Tests the determine_axis_roles function for correct output
        """
        mixed_roles = gpm.determine_axis_roles(mock_mixed_module)
        # test a: slave port is detected correctly
        self.assertIn("mock_slave_module", mixed_roles)
        self.assertEqual(mixed_roles["mock_slave_module"], "slave")
        # test b: master port is detected correctly
        self.assertIn("mock_master_module", mixed_roles)
        self.assertEqual(mixed_roles["mock_master_module"], "master")
        # test c: unknown AXIS port is not detected
        self.assertNotIn("mock_unknown_module", mixed_roles)

    def test_generate_port_mappings_for_module(self):
        """
        Tests the generate_port_mappings_for_module function for correct output
        """
        mock_file = Mock()
        mock_file.write = Mock()
        warnings = gpm.generate_port_mappings_for_module(mock_file, mock_generate_module, mock_generate_module_axis_roles)

        mock_file.write.assert_has_calls(mock_generate_module_port_mappings)
        self.assertTrue(warnings, "Warnings should be generated when there exists a value that cannot be inferred automatically.")

if __name__ == "__main__":
    unittest.main()