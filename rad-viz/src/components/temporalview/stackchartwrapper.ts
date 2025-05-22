import * as d3 from "d3";
import StackedChart from "./stackchart";
import { SVGSelection, StackBarOptions } from "./stackchart";
import { TemporalViewRendering } from "@/data";
import TemporalViewLegend from "./legend";

type TemporalViewDataArray = TemporalViewRendering[];

const option: StackBarOptions = {
  x: (d) => d.time,
  y: (d) => d.pkt_count,
  z: (d) => d.pkt_type, // or d.doc
  width: 0,
  height: 0,
  offset: d3.stackOffsetNone,
  xLabel: "Simulation Time", // runtime
  yLabel: "Total # of On-the-fly Flits in the NoC (Unit: 1 flit)",
  zDomain: undefined, // runtime
  colors: undefined, // runtime
  yFormat: "~s", // SI prefix and trims insignificant trailing zeros
};

export default class StackedChartWrapper {
  protected chart!: StackedChart;
  protected svg!: SVGSelection;
  protected brush!: d3.BrushBehavior<unknown>;
  protected data!: TemporalViewDataArray;
  protected storedTimeRange: [number, number];
  protected timeRangeChangedCallback: (l: number, r: number) => void;
  protected legend!: HTMLDivElement;

  constructor(timeRangeChangedCallback: (l: number, r: number) => void) {
    this.storedTimeRange = [0, 0];
    this.timeRangeChangedCallback = timeRangeChangedCallback;
  }

  loadData(data: TemporalViewDataArray, xAxisUnit: number /* # of cycles */) {
    const pktTypeDomain = new Array<string>();
    let maxTime: number = 0;
    const availableTimes = new Array<string>();
    data.forEach((d) => {
      if (pktTypeDomain.includes(d.pkt_type) === false) {
        pktTypeDomain.push(d.pkt_type);
      }
      maxTime = Math.max(maxTime, Number(d.time));
      availableTimes.push(d.time);
    });

    for (let i = 0; i <= maxTime; i++) {
      if (availableTimes.includes(`${i}`) === false) {
        data.push({
          time: `${i}`,
          pkt_type: pktTypeDomain[0],
          pkt_count: 0,
        });
      }
    }

    const groupedData = d3.group(
      data,
      (d) => d.time,
      (d) => d.pkt_type
    );
    // console.log(groupedData);

    this.data = [...data];

    groupedData.forEach((val, key) => {
      pktTypeDomain.forEach((domain) => {
        if (val.has(domain) == false) {
          this.data.push({
            time: key,
            pkt_type: domain,
            pkt_count: 0,
          });
        }
      });
    });

    this.data = this.data
      .sort(function (a, b) {
        return d3.ascending(Number(a.time), Number(b.time));
      })
      .sort(function (a, b) {
        return d3.ascending(a.pkt_type, b.pkt_type);
      });

    option.zDomain = pktTypeDomain;
    this.legend = new TemporalViewLegend().data(pktTypeDomain).node();
    option.colors =
      pktTypeDomain.length <= 2
        ? ["#2b83ba", "#d7191c"] // blue, red
        : d3.schemeSpectral[pktTypeDomain.length];

    option.xLabel = `Simulation Time (Unit: ${xAxisUnit} ns)`;
  }

  getSVGElement(width: number, height: number): SVGSVGElement {
    option.width = width;
    option.height = height;

    let chart = new StackedChart(this.data, option);
    let svg = chart.axis();
    svg.attr("id", "stacked-chart");
    chart.bar(svg);
    let brush = chart.brush(
      svg,
      (l, r) => {
        // console.log(`selected by brushing ${l}:${r}`);
        this.timeRangeChangedCallback(l, r);
        this.storedTimeRange = [l, r];
      },
      this.storedTimeRange
    );

    this.chart = chart;
    this.svg = svg;
    this.brush = brush;

    return this.svg.node()!;
  }

  getLegendDiv(): HTMLDivElement {
    return this.legend;
  }

  moveBrush(left: number, right: number) {
    this.storedTimeRange = [left, right];
    this.chart.moveBrush(this.svg, this.brush, this.storedTimeRange);
    console.log(`selected by program/keyboard ${left}:${right}`);
    this.timeRangeChangedCallback(left, right);
  }

  nextBrush(step?: number) {
    if (step === undefined) {
      step = 1;
    }
    const [left, right] = this.storedTimeRange;
    if (left + step >= 0) {
      this.moveBrush(left + step, right + step);
    }
  }
}

// function ColorScheme(lv: number): string {
//   // [0, 9] maps Blue-Yellow-Red color platte
//   return d3.interpolateReds(lv / 2000);
// }
