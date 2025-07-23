import { xmlToString } from "@/utils";
import MxGraphStyle from "./styleparser";

import { DiagramViewer } from "../diagrams";

interface MxCellAttributes {
  id: string;
  value: string;
  style: MxGraphStyle;
  vertex: string | null;
  connectable: string | null;
  edge: string | null;
  source: string | null;
  target: string | null;
  elementRef: Element;
}

export default class DiagramModel {
  protected doc: XMLDocument;
  protected templateDiagramString: string;
  //   protected anchors: Array<string>;
  //   protected anchorToVertexID: Map<string, string>;
  //   protected vertexIDToNumber: Map<string, number>;
  //   protected adjMatrix: Array<Array<string>>;
  protected vertexNameToVertexObject: Map<string, MxCellAttributes>;
  protected vertexIDToVertexName: Map<string, string>;
  protected vertexIDToVertexIndex: Map<string, number>;
  protected vertexNameToVertexIndex: Map<string, number>;
  protected adjMatrixToEdgeObject: Array<Array<MxCellAttributes | undefined>>;
  protected edgeAttrRefs: Array<MxCellAttributes>;
  protected vertexAttrRefs: Array<MxCellAttributes>;

  constructor(diagramString: string) {
    this.doc = new DOMParser().parseFromString(
      diagramString,
      "application/xml"
    ) as XMLDocument;

    // just duplicate the XML string for future online editing
    this.templateDiagramString = diagramString;

    // sanitize XML by removing `object` tags
    [...this.doc.getElementsByTagName("object")].forEach((el) => {
      el.children[0].id = el.id;
      el.children[0].setAttribute("value", el.getAttribute("label")!);
      el.replaceWith(el.children[0]);
    });
    // throw Error("Tag `object` in the diagram XML is not supported!");
    // console.log(xmlToString(this.doc));

    let cells = [...this.doc.getElementsByTagName("mxCell")];
    let cellAttrs = cells.map(
      (c) =>
        ({
          id: c.id,
          value: c.getAttribute("value")!,
          style: new MxGraphStyle(c.getAttribute("style")),
          vertex: c.getAttribute("vertex"),
          connectable: c.getAttribute("connectable"),
          edge: c.getAttribute("edge"),
          source: c.getAttribute("source"),
          target: c.getAttribute("target"),
          elementRef: c,
        } as MxCellAttributes)
    );

    // console.log(cellAttrs);

    let vertexAttrs = cellAttrs.filter((obj) => {
      return (
        obj.vertex === "1" &&
        obj.connectable !== "0" &&
        obj.value !== null && // able to be located. TODO: validate vertex/edge
        !obj.value.includes("text;")
      );
    });
    // console.log(vertexAttrs);

    let edgeAttrs = cellAttrs.filter((obj) => obj.edge === "1");
    // console.log(edgeAttrs);

    this.vertexIDToVertexName = new Map<string, string>();
    this.vertexIDToVertexIndex = new Map<string, number>();
    this.vertexNameToVertexIndex = new Map<string, number>();
    this.vertexNameToVertexObject = new Map<string, MxCellAttributes>();
    vertexAttrs.forEach((obj, index) => {
      if (this.vertexNameToVertexIndex.has(obj.value)) {
        console.error(`Duplicated vertex ${obj.value} occurs`);
      }
      this.vertexIDToVertexName.set(obj.id, obj.value);
      this.vertexIDToVertexIndex.set(obj.id, index);
      this.vertexNameToVertexIndex.set(obj.value, index);
      this.vertexNameToVertexObject.set(obj.value, obj);
    });

    this.adjMatrixToEdgeObject = new Array<
      Array<MxCellAttributes | undefined>
    >();

    let matrixSize = vertexAttrs.length;

    for (let i = 0; i < matrixSize; i++) {
      let row = new Array<MxCellAttributes | undefined>();
      for (let j = 0; j < matrixSize; j++) {
        row.push(undefined);
      }
      this.adjMatrixToEdgeObject.push(row);
    }

    edgeAttrs.forEach((obj) => {
      let sourceIndex = this.vertexIDToVertexIndex.get(obj.source!);
      let targetIndex = this.vertexIDToVertexIndex.get(obj.target!);
      // TODO: type check
      if (
        typeof this.adjMatrixToEdgeObject[sourceIndex!][targetIndex!] ===
        "undefined"
      ) {
        this.adjMatrixToEdgeObject[sourceIndex!][targetIndex!] = obj;
      }
    });
    // console.log(vertexAttrs);
    // console.log(edgeAttrs);

    this.edgeAttrRefs = edgeAttrs; // for resetting the style of all edges
    this.vertexAttrRefs = vertexAttrs; // for resetting the style of all vertexes
  }

  setEdgeBase(
    sourceVName: string,
    targetVName: string,
    action: (obj: MxCellAttributes) => boolean
  ): boolean {
    const sourceVIndex = this.vertexNameToVertexIndex.get(sourceVName);
    if (sourceVIndex === undefined) {
      console.warn(`there is no vertex ${sourceVName} in the diagram`);
      return false;
    }
    const targetVIndex = this.vertexNameToVertexIndex.get(targetVName);
    if (targetVIndex === undefined) {
      console.warn(`there is no vertex ${targetVName} in the diagram`);
      return false;
    }

    let obj = this.adjMatrixToEdgeObject[sourceVIndex!][targetVIndex!];
    if (obj === undefined) {
      console.warn(`there is no edge from ${sourceVName} to ${targetVName}`);
      return false;
    }

    return action(obj);
  }

  setEdgeStyle(
    sourceVName: string,
    targetVName: string,
    styleName: string,
    styleValue: string
  ): boolean {
    return this.setEdgeBase(sourceVName, targetVName, (obj) => {
      obj.style.setStyleAttribute(styleName, styleValue);
      obj.elementRef.setAttribute("style", obj.style.getStyleString());
      return true;
    });
  }

  setEdgeText(sourceVName: string, targetVName: string, text: string): boolean {
    return this.setEdgeBase(sourceVName, targetVName, (obj) => {
      obj.value = text;
      obj.elementRef.setAttribute("value", text);
      return true;
    });
  }

  setAllEdgeStyle(styleName: string, styleValue: string) {
    this.edgeAttrRefs.forEach((obj) => {
      obj.style.setStyleAttribute(styleName, styleValue);
      obj.elementRef.setAttribute("style", obj.style.getStyleString());
    });
  }

  resetAllEdgeStyle() {
    this.edgeAttrRefs.forEach((obj) => {
      obj.style.resetStyle();
      obj.elementRef.setAttribute("style", obj.style.getStyleString());
    });
  }

  setVertexBase(
    vertexName: string,
    action: (obj: MxCellAttributes) => boolean
  ): boolean {
    let obj = this.vertexNameToVertexObject.get(vertexName);
    if (obj === undefined) {
      console.warn(`there is no "${vertexName}" in the diagram`);
      return false;
    }
    return action(obj);
  }

  setVertexText(vertexName: string, text: string): boolean {
    return this.setVertexBase(vertexName, (obj) => {
      obj.value = text;
      obj.elementRef.setAttribute("value", text);
      return true;
    });
  }

  setVertexStyle(
    vertexName: string,
    styleName: string,
    styleValue: string
  ): boolean {
    return this.setVertexBase(vertexName, (obj) => {
      obj.style.setStyleAttribute(styleName, styleValue);
      obj.elementRef.setAttribute("style", obj.style.getStyleString());
      return true;
    });
  }

  setAllVertexStyle(styleName: string, styleValue: string) {
    this.vertexAttrRefs.forEach((obj) => {
      obj.style.setStyleAttribute(styleName, styleValue);
      obj.elementRef.setAttribute("style", obj.style.getStyleString());
    });
  }

  forEachEdge(callback: (sourceVName: string, targetVName: string) => void) {
    this.edgeAttrRefs.forEach((obj) => {
      const sourceVertexName = this.vertexIDToVertexName.get(obj.source!)!;
      const targetVertexName = this.vertexIDToVertexName.get(obj.target!)!;
      callback(sourceVertexName, targetVertexName);
    });
  }

  XMLDocumentRef() {
    return this.doc;
  }

  XMLString() {
    return xmlToString(this.doc)!;
  }

  XMLStringTemplateForEditing() {
    return this.templateDiagramString;
  }

  SVGElementObject() {
    const diagramViewer = new DiagramViewer(this.XMLDocumentRef());
    return diagramViewer.renderSVG(null, 1, 1) as SVGElement;
  }
}
