import os
import unittest
from scripts import generate_wrapper as gw

PROJECT_PATH = os.getcwd()

class GenerateWrapperTest(unittest.TestCase):
    """
    GenerateWrapperTest class to test the functions in generate_wrapper.py
    """

    def test_read_port_mappings(self):
        """
        Tests the read_port_mappings function for correct output
        """

        mock_port_map_file = PROJECT_PATH + "/test/wrapper-scripts/mock_port.map"
        (mappings, axis_role) = gw.read_port_mappings(mock_port_map_file)
        self.assertEqual(len(mappings), 1, "There should only be one module in the port map file")

        moduleMappings = mappings["mock_module"]
        self.assertEqual(len(moduleMappings), 4)
        self.assertIn(("sc_in", "1", "i1", "i1"), moduleMappings)
        self.assertIn(("sc_inout", "64", "io64", "io64"), moduleMappings)
        self.assertIn(("sc_out", "32", "o32", "o32"), moduleMappings)
        self.assertIn(("axis", "master", "axis_mock_module_interface_tdata", "axis_mock_module_interface.tdata"), moduleMappings)

    def test_read_port_mappings_incomplete(self):
        """
        Tests the read_port_mappings function when manual verification is required
        """

        mock_port_map_file = PROJECT_PATH + "/test/wrapper-scripts/mock_port_incomplete.map"
        self.assertRaises(ValueError, gw.read_port_mappings, mock_port_map_file)

if __name__ == "__main__":
    unittest.main()