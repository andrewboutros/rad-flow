import { SpatialViewRendering } from "@/data";

import DiagramModel from "./diagram";
import SpatialViewModelAddon from "./addons/addons";
import { MapString2D, RenderDataUsingD3 } from "./types";

import SpatialViewLegendCheckboxes from "./legend";

import * as d3 from "d3";

export default class SpatialViewModel {
  diagramModel: DiagramModel;
  pathAttrDMappingTable: MapString2D<[string, string]>;
  diagramSVG: SVGElement;
  addonsRegisterTable: Map<string, SpatialViewModelAddon>;
  weightFilterOutTable: Map<number, boolean>;
  replayRenderData: SpatialViewRendering[];

  constructor(
    diagramModel: DiagramModel,
    pathAttrDMappingTable: MapString2D<[string, string]>
  ) {
    this.diagramModel = diagramModel;
    this.diagramSVG = diagramModel.SVGElementObject();
    this.pathAttrDMappingTable = pathAttrDMappingTable;
    this.addonsRegisterTable = new Map<string, SpatialViewModelAddon>();
    this.weightFilterOutTable = new Map<number, boolean>();
    this.replayRenderData = new Array<SpatialViewRendering>();

    const svg = d3.select(this.diagramSVG);

    const svgHeight = parseInt(svg.attr("height"));
    const paneHeight =
      document.getElementById("spatial-view-body")!.parentElement!
        .clientHeight!;
    const windowHeight = window.innerHeight;

    svg.attr("width", "100vw").attr("height", "100vh");
    const g = svg.select("g");
    const zoom = d3.zoom().on("zoom", (e) => g.attr("transform", e.transform));

    // console.log(svgHeight, paneHeight, windowHeight)
    // TODO: add slider for the initial scale of the spatial view
    svg.call(zoom.scaleBy as any, Number(svgHeight / windowHeight));
    svg.call(zoom.translateBy as any, 0, (paneHeight - windowHeight) / 2);
    svg.call(zoom as any);
    svg.on("dblclick.zoom", null); // disable the double click event for zooming

    svg.selectAll("path").lower();

    svg.selectAll("rect").each(function () {
      const sibling = d3.select((this as Element).nextElementSibling);
      const name = sibling.select("text").text();
      d3.select(this).attr("id", `vertex-${name}`);
    });
  }

  registerAddon(name: string, addon: SpatialViewModelAddon) {
    this.addonsRegisterTable.set(name, addon);
  }

  feed(renderData: SpatialViewRendering[]): SVGElement {
    this.replayRenderData = renderData;

    const cleanup = () => {
      this.addonsRegisterTable.forEach((addon) => {
        addon.reset();
      });
    };
    cleanup();

    const pathAttrDToRender = new Map<string, RenderDataUsingD3>();
    renderData = renderData.filter(
      (d) => this.weightFilterOutTable.get(d.norm) === false
    );
    renderData.forEach((d) => {
      const obj = this.pathAttrDMappingTable.get(d.src)!.get(d.dst)!;
      pathAttrDToRender.set(obj[0 /*edge*/], {
        sourceVName: d.src,
        targetVName: d.dst,
        renderType: "edge",
        fillColor: "none",
        strokeColor: d.color,
        strokeWidth: "3.5", // TODO: use customized width
      });
      pathAttrDToRender.set(obj[1 /*arrow*/], {
        sourceVName: d.src,
        targetVName: d.dst,
        renderType: "arrow",
        fillColor: d.color,
        strokeColor: d.color,
        strokeWidth: "3.5",
      });
    });

    const svg = d3.select(this.diagramSVG);
    svg.on("click", cleanup);
    svg
      .selectAll("path")
      .attr("fill", function () {
        const attrD = d3.select(this).attr("d") as string;
        if (pathAttrDToRender.has(attrD)) {
          return pathAttrDToRender.get(attrD)!.fillColor;
        } else {
          return "none";
        }
      })
      .attr("stroke", function () {
        const attrD = d3.select(this).attr("d") as string;
        if (pathAttrDToRender.has(attrD)) {
          return pathAttrDToRender.get(attrD)!.strokeColor;
        } else {
          return "none";
        }
      })
      .attr("stroke-width", function () {
        const attrD = d3.select(this).attr("d") as string;
        if (pathAttrDToRender.has(attrD)) {
          return pathAttrDToRender.get(attrD)!.strokeWidth;
        } else {
          return "1";
        }
      });

    this.addonsRegisterTable.forEach((addon) => {
      addon.effect(svg, pathAttrDToRender, renderData);
    });

    return svg.node()!;
  }

  legend(scale: string /*special format*/) {
    function formatNumberUsingD3(x: number): string {
      const format = d3.format(".3s")(x);
      const len = format.length;
      const trans = Number(format);
      if (Number.isNaN(trans) === true) {
        const prefix = format.substring(0, len - 1);
        return `${Number(prefix)}${format.charAt(len - 1)}`;
      } else {
        return `${trans}`;
      }
    }

    const prefix = scale.charAt(0) == "^" ? ">=" : ""; // `^10`, ` 99`, ` 100`, ...
    const scaleNumber = Number(scale.substring(1));
    const upperLabelTable = new Map<number, string>();
    Array.from(Array(9).keys()).forEach((x) => {
      upperLabelTable.set(
        x,
        formatNumberUsingD3(Math.floor((x + 1) * scaleNumber))
      );
    });
    upperLabelTable.set(8, `${prefix}${upperLabelTable.get(8)!}`);

    return SpatialViewLegendCheckboxes(
      this.weightFilterOutTable,
      upperLabelTable,
      () => {
        this.feed(this.replayRenderData);
      }
    );
  }
}
