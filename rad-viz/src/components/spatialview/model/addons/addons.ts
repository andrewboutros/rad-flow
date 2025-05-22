import { RenderDataUsingD3, D3SVGSelection } from "../types";

import { SpatialViewRendering } from "@/data";

export default abstract class SpatialViewModelAddon {
  constructor() {}

  abstract effect(
    svg: D3SVGSelection,
    pathAttrDToRender: Map<string, RenderDataUsingD3>,
    renderData: SpatialViewRendering[]
  ): void;

  abstract reset(): void;
}
