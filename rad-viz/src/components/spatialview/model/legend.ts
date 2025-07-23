import * as d3 from "d3";

interface CheckboxRenderData {
  level: number;
  upper: string;
}

const NumLevels = 9;

const config = {
  boxWidth: 40,
  boxHeight: 20,
  boxPadding: 0,
  yPadding: 50,
};

function colorOf(level: number): string {
  return d3.schemeOrRd[NumLevels][level];
}

export default function SpatialViewLegendCheckboxes(
  uncheckedTableRef: Map<number, boolean>,
  upperLabelTable: Map<number, string>,
  refreshSpatialView: () => void
) {
  const svg = d3
    .create("svg")
    .attr(
      "viewBox",
      `0 0 ${config.boxWidth * 2.5} ${
        config.boxHeight * (NumLevels + 1) /* unit label at the top */ +
        2 * config.yPadding
      }`
    );
  const data = new Array<CheckboxRenderData>();
  Array.from(Array(NumLevels).keys()).forEach((x) => {
    uncheckedTableRef.set(x, false);
    data.push({
      level: x,
      upper: upperLabelTable.get(x)!,
    });
  });

  const sizeY = config.boxHeight + config.boxPadding;
  const rectID = (level: number) => {
    return `spatial-view-legend-rect-${level}`;
  };
  const mapY = (level: number) => {
    return (NumLevels - 1 - level) * sizeY + config.yPadding;
  };
  const mapLevel = (y: number) => {
    return NumLevels - 1 - Math.floor((y - config.yPadding) / sizeY);
  };

  const g = svg.append("g");

  // Box
  g.selectAll("rect")
    .data(data)
    .join(
      function (enter) {
        return enter
          .append("rect")
          .attr("id", function (d) {
            return rectID(d.level);
          })
          .attr("width", config.boxWidth)
          .attr("height", config.boxHeight)
          .attr("stroke", "#ccc")
          .attr("stroke-width", "0.25%");
      },
      function (update) {
        return update;
      },
      function (exit) {
        return exit.on("end", function () {
          d3.select(this).remove();
        });
      }
    )
    .attr("x", 0)
    .attr("y", function (d) {
      return mapY(d.level);
    })
    .attr("fill", (d) => {
      return !uncheckedTableRef.get(d.level) ? colorOf(d.level) : "none";
    });

  svg.on("click", (event) => {
    const [x, y] = d3.pointer(event);
    if (x <= config.boxWidth) {
      const level = mapLevel(y);
      const rect = d3.select(`#${rectID(level)}`);
      const prevUnchecked = uncheckedTableRef.get(level)!;
      rect.transition().attr("fill", prevUnchecked ? colorOf(level) : "none");
      uncheckedTableRef.set(level, !prevUnchecked);
      console.log(uncheckedTableRef);
      refreshSpatialView();
    }
  });

  // Label
  g.selectAll(".spatial-view-legend-label")
    .data(data.concat({ level: -1, upper: "0" }))
    .join(
      function (enter) {
        return enter
          .append("text")
          .attr("class", "spatial-view-legend-label")
          .attr("dominant-baseline", "middle")
          .attr("text-anchor", "start")
          .attr("fill", "black")
          .attr("font-size", "0.85em")
          .attr("font-weight", "normal");
      },
      function (update) {
        return update;
      },
      function (exit) {
        return exit.on("end", function () {
          d3.select(this).remove();
        });
      }
    )
    .attr("x", config.boxWidth + 6)
    .attr("y", function (d) {
      return mapY(d.level);
    })
    .text(function (d) {
      return d.upper;
    });

  // Unit Label at the top
  let maxLabelTextLength: number = 0;
  data.forEach((d) => {
    const length = d.upper.includes(".") ? d.upper.length - 1 : d.upper.length;
    maxLabelTextLength = Math.max(maxLabelTextLength, length);
  });
  const labelPadding = [0, 10.5, 14.75, 18.0, 22.0];
  maxLabelTextLength = maxLabelTextLength == 5 ? 4 : maxLabelTextLength;
  g.append("text")
    .attr("class", "spatial-view-legend-label")
    .attr("dominant-baseline", "middle")
    .attr("text-anchor", "middle")
    .attr("fill", "black")
    .attr("font-size", "0.85em")
    .attr("font-weight", "normal")
    .attr("x", config.boxWidth + labelPadding[maxLabelTextLength])
    .attr("y", mapY(NumLevels))
    .text("# of Flits");

  return svg.node();
}
