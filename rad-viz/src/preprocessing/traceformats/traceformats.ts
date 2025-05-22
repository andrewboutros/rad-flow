import TransformFromRADSimTraceFormat from "./rad-sim";
import TransformFromLowLevelTraceFormat from "./low-level";

import { LowLevelInputTrace } from "@/preprocessing/preprocessing";

export default function TransformIntoLowLevelTraceFormat(
  fmt: string,
  traces: string,
  aggregatedTimeUnit?: number
): Array<LowLevelInputTrace> {
  // TODO: validation logic for the traces provided
  if (fmt === "rad-sim") {
    return TransformFromRADSimTraceFormat(traces, aggregatedTimeUnit);
  } else if (fmt === "low-level") {
    return TransformFromLowLevelTraceFormat(traces, aggregatedTimeUnit);
  } else {
    return [];
  }
  // TODO: return CSV object instead of raw string
}
