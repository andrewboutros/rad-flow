import * as d3 from "d3";

type DetachedDivNode = d3.Selection<HTMLDivElement, undefined, null, undefined>;

interface CheckBoxOptions {
  label: string;
  color?: string;
  boxWidth?: number; // px
  boxHeight?: number; // px
  labelAlign?: "middle" | "top" | "bottom";
}

class ColoredCheckbox {
  protected color!: string;
  protected div: DetachedDivNode;

  constructor(id?: string) {
    this.div = d3.create("div");
    if (id !== undefined) {
      this.div.attr("id", id);
    }
    this.div
      .style("margin", "0.25rem")
      .style("display", "inline-block")
      .style("font-size", "0.85em");
  }

  option(opt: CheckBoxOptions): this {
    this.color = opt.color === undefined ? "blue" : opt.color;
    const width = opt.boxWidth === undefined ? 21 : opt.boxWidth;
    const height = opt.boxHeight === undefined ? 21 : opt.boxHeight;
    const labelAlign = opt.labelAlign === undefined ? "middle" : opt.labelAlign;

    let svg = this.div
      .append("svg")
      .attr("width", width)
      .attr("height", height)
      .style("margin", "0.25em")
      .style("display", "inline-block");

    svg
      .append("rect")
      .attr("width", width)
      .attr("height", height)
      .attr("stroke", "grey")
      .attr("stroke-width", 3)
      .attr("fill", "none")
      .attr("rx", 3)
      .attr("ry", 3)
      .property("checked", false);

    const label = this.div
      .append("label")
      .text(opt.label)
      .style("margin", "0.25em")
      .style("position", "relative")
      .style("vertical-align", "initial");
    if (labelAlign === "middle") {
      // by default
    } else if (labelAlign === "bottom") {
      label.style("top", `${height / 2.0}px`);
    } else if (labelAlign === "top") {
      label.style("bottom", `${height / 2.0}px`);
    }

    return this;
  }

  event(handle: (status: boolean) => any): this {
    const color = this.color;
    this.div.select("svg").on("click", function () {
      let rect = d3.select(this).select("rect");
      if (rect.property("checked") === false) {
        rect.property("checked", true).transition().attr("fill", color);
        handle(true);
      } else {
        rect.property("checked", false).transition().attr("fill", "none");
        handle(false);
      }
    });
    return this;
  }

  static(checked: boolean): this {
    const rect = this.div.select("rect");
    if (checked) {
      rect.property("checked", true).transition().attr("fill", this.color);
    } else {
      rect.property("checked", false).transition().attr("fill", "none");
    }
    return this;
  }

  switch(checked: boolean): this {
    // change to last state, then click it to current state
    this.static(!checked);
    this.div.select("svg").dispatch("click");
    return this;
  }

  rename(label: string): this {
    this.div.select("label").text(label);
    return this;
  }

  node() {
    return this.div.node()!;
  }
}

export default class TemporalViewLegend {
  protected div: DetachedDivNode;

  constructor() {
    this.div = d3.create("div");
    this.div
      .style("position", "absolute")
      .style("right", "1em")
      .style("top", "inherit");
  }

  data(domain: Array<string>) {
    domain.forEach((group, i) => {
      const colorScheme: readonly string[] =
        domain.length <= 2
          ? ["#2b83ba", "#d7191c"] // TODO: generic
          : d3.schemeSpectral[domain.length];
      const box = new ColoredCheckbox()
        .option({
          label: group,
          color: colorScheme[i],
        })
        .event((val) => {
          console.log(`${group} clicked to ${val}`);
        })
        .static(true);
      this.div.append(() => box.node());
    });

    return this;
  }

  node() {
    return this.div.node()!;
  }
}
