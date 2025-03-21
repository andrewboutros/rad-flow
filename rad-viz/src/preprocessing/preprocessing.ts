import * as d3 from "d3";
import { TemporalViewRendering, SpatialViewRendering } from "@/data";

export interface LowLevelInputTrace {
  time: string;
  grp_id: string;
  pkt_src: string;
  pkt_dst: string;
  pkt_type?: string;
  comment?: string;
}

export default class Preprocessing {
  timeMergedData: { [time: string]: Array<LowLevelInputTrace> };
  maxTime: number;

  constructor(lowLevelInputTrace: Array<LowLevelInputTrace>) {
    this.timeMergedData = {};
    this.maxTime = 0;

    lowLevelInputTrace.forEach((d) => {
      if (d.time in this.timeMergedData) {
        this.timeMergedData[d.time].push(d);
      } else {
        this.timeMergedData[d.time] = new Array<LowLevelInputTrace>(d);
        this.maxTime = Math.max(this.maxTime, Number(d.time));
      }
    });

    // console.log(this.timeMergedData);
  }

  getColorFromEdgeValue(value: number): string {
    return d3.schemeOrRd[9][Math.floor(value)];
  }

  getRenderingJsonAtTimeRange(
    begin: number,
    end: number,
    mapEdgeValueToTheRangeFrom0To8: (value: number) => number
  ): Array<SpatialViewRendering> {
    const vertexSet = new Set<string>();

    let emptyResult: boolean = true;
    for (let time = begin; time < end; time++) {
      if (`${time}` in this.timeMergedData == true) {
        emptyResult = false;
        break;
      }
    }
    if (emptyResult) {
      return [];
    }

    for (let time = begin; time < end; time++) {
      if (`${time}` in this.timeMergedData == true) {
        this.timeMergedData[`${time}`].forEach((d) => {
          vertexSet.add(d.pkt_src);
          vertexSet.add(d.pkt_dst);
        });
      }
    }

    let vIndex = 0;
    const vNameToIndex = new Map<string, number>();
    const vIndexToName = new Map<number, string>();
    vertexSet.forEach((v) => {
      vNameToIndex.set(v, vIndex);
      vIndexToName.set(vIndex, v);
      vIndex += 1;
    });

    const size = vertexSet.size;
    const edgeValueMap = new Array<Array<number>>(size)
      .fill([])
      .map(() => new Array<number>(size).fill(0));
    const groupListMap = new Array<Array<Array<string>>>(size)
      .fill([])
      .map(() =>
        new Array<Array<string>>(size).fill([]).map(() => new Array<string>())
      );

    for (let time = begin; time < end; time++) {
      if (`${time}` in this.timeMergedData == true) {
        this.timeMergedData[`${time}`].forEach((d) => {
          const srcIdx = vNameToIndex.get(d.pkt_src)!;
          const dstIdx = vNameToIndex.get(d.pkt_dst)!;
          edgeValueMap[srcIdx][dstIdx] += 1;
          groupListMap[srcIdx][dstIdx].push(d.grp_id);
        });
      }
    }

    const resultJson: Array<SpatialViewRendering> = new Array();
    edgeValueMap.forEach((row, i) =>
      row.forEach((edgeValue, j) => {
        if (edgeValue > 0) {
          const norm = mapEdgeValueToTheRangeFrom0To8(edgeValue);
          resultJson.push({
            src: vIndexToName.get(i)!,
            dst: vIndexToName.get(j)!,
            color: this.getColorFromEdgeValue(norm),
            norm: norm,
            weight: edgeValue,
            groups: groupListMap[i][j]!,
          });
        }
      })
    );

    return resultJson;
  }

  getRenderingJsonAtTimeOf(
    time: number,
    mapEdgeValueToTheRangeFrom0To8: (value: number) => number
  ): Array<SpatialViewRendering> {
    return this.getRenderingJsonAtTimeRange(
      time,
      time + 1,
      mapEdgeValueToTheRangeFrom0To8
    );
  }

  getTemporalViewRenderingJson(): Array<TemporalViewRendering> {
    const resultJson = new Array<TemporalViewRendering>();
    for (let i = 0; i <= this.maxTime; i++) {
      const reduceMap = new Map<string, number>(); // pkt_type -> pkt_count
      if (`${i}` in this.timeMergedData) {
        this.timeMergedData[`${i}`].forEach((t) => {
          const key = t.pkt_type === undefined ? "undefined" : t.pkt_type;
          if (reduceMap.has(key)) {
            reduceMap.set(key, reduceMap.get(key)! + 1);
          } else {
            reduceMap.set(key, 1);
          }
        });
      }
      // console.log(i, reduceMap.size);
      reduceMap.forEach((val, key) => {
        resultJson.push({
          time: `${i}`,
          pkt_type: key,
          pkt_count: val,
        });
      });
      // TODO: move it to stackedchart module to handle
      // if (reduceMap.size == 0) {
      //   resultJson.push({
      //     time: `${i}`,
      //     pkt_type: 'undefined',
      //     pkt_count: 0,
      //   })
      // }
    }
    // console.log(resultJson);
    return resultJson;
  }
}
