export type MapString2D<T> = Map<string, Map<string, T>>;

export interface RenderDataUsingD3 {
  sourceVName: string;
  targetVName: string;
  renderType: "edge" | "arrow";
  fillColor: string;
  strokeColor: string;
  strokeWidth: string;
}

export type D3SVGSelection = d3.Selection<SVGElement, unknown, null, undefined>;
