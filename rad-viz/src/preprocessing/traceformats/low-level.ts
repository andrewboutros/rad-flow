import Papa from "papaparse";

import { LowLevelInputTrace } from "@/preprocessing/preprocessing";

export default function TransformFromLowLevelTraceFormat(
  csvString: string,
  aggregatedTimeUnit?: number
) {
  const data = Papa.parse(csvString, {
    header: true,
    skipEmptyLines: true,
    transformHeader: (header: string) => header.trim(),
  }).data as Array<LowLevelInputTrace>;

  // TODO: remove quotes, e.g., in the `comment` field
  data.forEach((d) => {
    d.time = d.time.trim();
    d.grp_id = d.grp_id !== undefined ? d.grp_id.trim() : "undefined_group";
    d.pkt_src = d.pkt_src.trim();
    d.pkt_dst = d.pkt_dst.trim();
    if (d.pkt_type !== undefined) {
      d.pkt_type = d.pkt_type.trim();
    }
    if (d.comment !== undefined) {
      d.comment = d.comment.trim();
    }

    if (aggregatedTimeUnit !== undefined) {
      d.time = `${Math.floor(parseInt(d.time, 10) / aggregatedTimeUnit)}`;
    }
  });

  return data;
}
