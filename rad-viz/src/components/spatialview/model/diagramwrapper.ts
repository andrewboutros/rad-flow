import DiagramModel from "./diagram";

import { SpatialViewRendering, SpatialViewModuleNoCPlacement } from "@/data"

export default class DiagramModelWrapper {
  public diagramModel: DiagramModel;

  constructor(diagramXMLString: string) {
    this.diagramModel = new DiagramModel(diagramXMLString);
  }

  edgeEffects(edgeRenderData: SpatialViewRendering[]) {
    this.diagramModel.resetAllEdgeStyle();
    edgeRenderData.every((edge) => {
      let valid = true;
      valid = valid && this.diagramModel.setEdgeStyle(edge.src, edge.dst, "strokeColor", edge.color);
      valid = valid && this.diagramModel.setEdgeStyle(edge.src, edge.dst, "strokeWidth", "3");
      // TODO: diagramModel.value.setEdgeText(edge.src, edge.dst, edge.text);
      if (!valid) {
        alert(`link from ${edge.src} to ${edge.dst} is invalid`)
      }
      return valid;
    })
  }

  vertexEffects(vertexRenderData: SpatialViewModuleNoCPlacement[]) {
    vertexRenderData.every((vertex) => {
      const valid = this.diagramModel.setVertexText(vertex.node_id, vertex.module_name);
      if (!valid) {
        alert(`Node ID ${vertex.node_id} in the placement file is invalid`);
      }
      return valid;
    });
  }

  renderSVG() {
    return this.diagramModel.SVGElementObject();
  }

  XMLStringTemplateForEditing() {
    return this.diagramModel.XMLStringTemplateForEditing();
  }
}
