import * as d3 from "d3";

import SpatialViewModelAddon from "./addons";
import { D3SVGSelection, RenderDataUsingD3 } from "../types";

import {
  OptionalTrace,
  SpatialViewModuleNoCPlacement,
  SpatialViewRendering,
} from "@/data";

export default class FlylineIndicator extends SpatialViewModelAddon {
  protected flylineSet: Set<string> /*`src module name => dst module name`*/;
  protected placementMappingTable: Map<string, string>;
  protected transactionDirectionMappingTable: Map<
    string /*trans. id*/,
    [string /*src*/, string /*dst*/]
  >;

  constructor(
    modulePlacementData: SpatialViewModuleNoCPlacement[],
    optionalTraceData: OptionalTrace[]
  ) {
    super();
    this.flylineSet = new Set<string>();
    this.placementMappingTable = new Map<string, string>();
    modulePlacementData.forEach((vertex) => {
      this.placementMappingTable.set(vertex.node_id, vertex.module_name);
    });
    this.transactionDirectionMappingTable = new Map<string, [string, string]>();
    optionalTraceData.forEach((d) => {
      if (this.transactionDirectionMappingTable.has(d.id)) {
        console.error(
          "The input stats.csv not valid - has duplicated `id` field"
        );
      } else {
        this.transactionDirectionMappingTable.set(d.id, [d.src, d.dst]);
      }
    });
  }

  effect(
    svg: D3SVGSelection,
    pathAttrDToRender: Map<string, RenderDataUsingD3>,
    renderData: SpatialViewRendering[]
  ): void {
    const flylineSet = this.flylineSet;
    const placementMappingTable = this.placementMappingTable;
    const transactionDirectionMappingTable =
      this.transactionDirectionMappingTable;
    const transactionIDMappingTable = new Map<
      string,
      Map<string, Array<string>>
    >();
    renderData.forEach((d) => {
      if (transactionIDMappingTable.has(d.src)) {
        transactionIDMappingTable.get(d.src)!.set(d.dst, d.groups);
      } else {
        transactionIDMappingTable.set(
          d.src,
          new Map<string, Array<string>>([[d.dst, d.groups]])
        );
      }
    });

    svg
      .selectAll("path")
      .on("mouseover", function () {
        const attrD = d3.select(this).attr("d") as string;
        if (pathAttrDToRender.has(attrD)) {
          const obj = pathAttrDToRender.get(attrD)!;
          const hoverStrokeWidth = Number(obj.strokeWidth) * 2.25;
          d3.select(this).attr("stroke-width", hoverStrokeWidth);

          const sibling =
            obj.renderType === "arrow"
              ? d3.select((this as Element).nextElementSibling)
              : d3.select((this as Element).previousElementSibling);
          sibling.attr("stroke-width", hoverStrokeWidth);
        }
      })
      .on("mouseout", function () {
        const attrD = d3.select(this).attr("d") as string;
        if (pathAttrDToRender.has(attrD)) {
          const obj = pathAttrDToRender.get(attrD)!;
          d3.select(this).attr("stroke-width", obj.strokeWidth);

          const sibling =
            obj.renderType === "arrow"
              ? d3.select((this as Element).nextElementSibling)
              : d3.select((this as Element).previousElementSibling);
          sibling.attr("stroke-width", obj.strokeWidth);
        }
      })
      .on("click", function (event) {
        if (transactionDirectionMappingTable.size === 0) {
          console.warn(
            `For flyline indicator feature, please paste the \`stats.csv\` file generated from the RAD-Sim into the RAD-Viz "Input Optional Traces" portal.`
          );
          return;
        }
        const attrD = d3.select(this).attr("d") as string;
        if (pathAttrDToRender.has(attrD)) {
          const obj = pathAttrDToRender.get(attrD)!;
          const [src, dst] = [obj.sourceVName, obj.targetVName];
          console.log(
            `Get flyline indicator from the flits which are transferring from vertex ID ${src} to ${dst}`
          );

          const uniqueGroups = new Set<string>();
          transactionIDMappingTable
            .get(src)!
            .get(dst)!
            .forEach((group) => {
              uniqueGroups.add(group);
            });

          const flyline = new Array<[string, string]>();
          Array.from(uniqueGroups).forEach((id) => {
            const [srcID, dstID] = transactionDirectionMappingTable.get(id)!;
            const srcModuleName = placementMappingTable.get(srcID)!;
            const dstModuleName = placementMappingTable.get(dstID)!;
            const key = `${srcModuleName} => ${dstModuleName}`;
            if (!flylineSet.has(key)) {
              flylineSet.add(key);
              flyline.push([srcModuleName, dstModuleName]);
            }
          });

          flyline.forEach(([src, dst]) => {
            const srcNode = svg.select(`#vertex-${src}`);
            const dstNode = svg.select(`#vertex-${dst}`);
            const xs =
              Number(srcNode.attr("x")) + Number(srcNode.attr("width")) / 2;
            const ys =
              Number(srcNode.attr("y")) + Number(srcNode.attr("height")) / 2;
            const xd =
              Number(dstNode.attr("x")) + Number(dstNode.attr("width")) / 2;
            const yd =
              Number(dstNode.attr("y")) + Number(dstNode.attr("height")) / 2;
            svg
              .select("g")
              .append("path")
              .attr("d", generateArrow(xs, ys, xd, yd, 14, 0, 0))
              .attr("class", "flyline")
              .style("fill", "none")
              .style("stroke", "grey")
              .style("stroke-dasharray", "5 5")
              .style("stroke-width", "4.5");
          });
        }
        event.stopPropagation();
      });
  }

  reset() {
    d3.selectAll(".flyline").remove();
    this.flylineSet.clear();
  }
}

function generateArrow(
  x1: number,
  y1: number,
  x2: number,
  y2: number,
  flangeSize: number,
  padding1: number,
  padding2: number
) {
  const dx = x2 - x1;
  const dy = y2 - y1;
  const length = Math.sqrt(dx * dx + dy * dy);

  let multiplier1 = padding1 / length;
  const dx1 = dx * multiplier1;
  const dy1 = dy * multiplier1;

  let multiplier2 = padding2 / length;
  const dx2 = dx * multiplier2;
  const dy2 = dy * multiplier2;

  var px = y1 - y2;
  var py = x2 - x1;
  let plength = Math.sqrt(px * px + py * py);
  let pmultiplier = flangeSize / plength;

  const px1 = px * pmultiplier;
  const py1 = py * pmultiplier;

  const sx = dx * pmultiplier;
  const sy = dy * pmultiplier;

  const a1 = x1 + dx1;
  const b1 = y1 + dy1;
  const a2 = x2 - dx2;
  const b2 = y2 - dy2;

  return `
      M${a1}, ${b1}
      L${a2}, ${b2}
      M${a2 + px1 - sx}, ${b2 + py1 - sy}
      L${a2}, ${b2}
      L${a2 - px1 - sx}, ${b2 - py1 - sy}
    `;
}
