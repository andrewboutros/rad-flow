import traceFlitsExampleCSV from "../../rad-sim/sim/flit_traces.csv?raw";
import traceStatsExampleCSV from "../../rad-sim/sim/stats.csv?raw";
import placementExampleCSV from "../../rad-sim/example-designs/mlp/mlp.place?raw";
import diagramExampleXML from "../example/mesh4x4.drawio?raw";

interface AnnotatedModule {
  moduleName: string;
  groupName: string;
}

function annotatePlacement(inputData: string): string {
  const nodePositionMap: Record<number, string> = {};
  const lines = inputData.trim().split("\n");

  for (const line of lines) {
    const parts = line.trim().split(/\s+/);
    if (parts.length >= 3) {
      // Extract node name and position (3rd column)
      const nodeName = parts[0];
      const position = parseInt(parts[2], 10);
      nodePositionMap[position] = nodeName;
    }
  }

  // Define mapping rules to convert from first format to second format
  const formatMapping: Record<string, AnnotatedModule> = {
    // Layer nodes
    layer0_mvm0: { moduleName: "L0M0", groupName: "L0" },
    layer0_mvm1: { moduleName: "L0M1", groupName: "L0" },
    layer0_mvm2: { moduleName: "L0M2", groupName: "L0" },
    layer0_mvm3: { moduleName: "L0M3", groupName: "L0" },
    layer1_mvm0: { moduleName: "L1M0", groupName: "L1" },
    layer1_mvm1: { moduleName: "L1M1", groupName: "L1" },
    layer1_mvm2: { moduleName: "L1M2", groupName: "L1" },
    layer2_mvm0: { moduleName: "L2M0", groupName: "L2" },
    layer2_mvm1: { moduleName: "L2M1", groupName: "L2" },
    layer3_mvm0: { moduleName: "L3M0", groupName: "L3" },
    layer3_mvm1: { moduleName: "L3M1", groupName: "L3" },
    // Dispatchers and collectors
    input_dispatcher0: { moduleName: "D0", groupName: "D" },
    input_dispatcher1: { moduleName: "D1", groupName: "D" },
    input_dispatcher2: { moduleName: "D2", groupName: "D" },
    input_dispatcher3: { moduleName: "D3", groupName: "D" },
    output_collector: { moduleName: "C", groupName: "C" },
  };

  const outputLines: string[] = [];
  outputLines.push("node_id, module_name, group_name");

  for (let position = 0; position < 16; position++) {
    // Assuming positions are 0-15
    if (position in nodePositionMap) {
      const nodeName = nodePositionMap[position];
      if (nodeName in formatMapping) {
        const { moduleName, groupName } = formatMapping[nodeName];
        outputLines.push(`${position}, ${moduleName}, ${groupName}`);
      }
    }
  }

  return outputLines.join("\n");
}

export const traceFlitsExampleCSVString = traceFlitsExampleCSV;
export const traceStatsExampleCSVString = traceStatsExampleCSV;
export const placementExampleCSVString = annotatePlacement(placementExampleCSV);
export const diagramExampleXMLString = diagramExampleXML;
