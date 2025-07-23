import DiagramModel from "./diagram";
import SpatialViewModel from "./spatialview";

import * as d3 from "d3";

function getSVGPathAttrD(diagramModel: DiagramModel, src: string, dst: string) {
  diagramModel.setAllEdgeStyle("strokeColor", "#ffffff"); // magic number for diffing
  diagramModel.setEdgeStyle(src, dst, "strokeColor", "#0f0f0f"); // magic number for diffing

  const svg = d3.select(diagramModel.SVGElementObject());

  const edgeAndArrow = svg.selectAll("path").filter(function () {
    return d3.select(this).attr("stroke") == "#0f0f0f";
  });

  const edge = edgeAndArrow.filter(function () {
    return d3.select(this).attr("pointer-events") == "stroke";
  });
  const arrow = edgeAndArrow.filter(function () {
    return d3.select(this).attr("pointer-events") == "all";
  });

  return [edge.attr("d") as string, arrow.attr("d") as string];
}

export default function TransformDiagramModelToSpatialViewModel(
  diagramModel: DiagramModel
) {
  const pathAttrDMappingTable = new Map<
    string,
    Map<string, [string, string]>
  >();

  diagramModel.forEachEdge((src, dst) => {
    const pathAttrD = getSVGPathAttrD(diagramModel, src, dst) as [
      string,
      string
    ];
    if (pathAttrDMappingTable.has(src)) {
      pathAttrDMappingTable.get(src)!.set(dst, pathAttrD);
    } else {
      pathAttrDMappingTable.set(
        src,
        new Map<string, [string, string]>([[dst, pathAttrD]])
      );
    }
  });

  diagramModel.resetAllEdgeStyle();

  return new SpatialViewModel(diagramModel, pathAttrDMappingTable);
}

// TODO: verification logic for NoC arch. diagram
