import Papa from "papaparse";

import { LowLevelInputTrace } from "@/preprocessing/preprocessing";

interface RADSimTrace {
  t_trace: string;
  link_src_router: string;
  link_dest_router: string;
  flit_id: string;
  flit_type: string;
  is_channel_output: string;
}

export default function TransformFromRADSimTraceFormat(
  csvString: string,
  aggregatedTimeUnit?: number
) {
  const data = Papa.parse(csvString, {
    header: true,
    skipEmptyLines: true,
    transformHeader: (header: string) => header.trim(),
  }).data as Array<RADSimTrace>;

  const result = new Array<LowLevelInputTrace>();

  data.forEach((d) => {
    const tr: LowLevelInputTrace = {
      time: d.t_trace.trim(),
      grp_id: d.flit_id.trim(),
      pkt_src: d.link_src_router.trim(),
      pkt_dst: d.link_dest_router.trim(),
      pkt_type: d.flit_type.trim(),
    };

    if (parseInt(tr.pkt_src, 10) != -1 && parseInt(tr.pkt_dst, 10) != -1) {
      const timeInt = parseInt(tr.time, 10);
      if (aggregatedTimeUnit !== undefined) {
        tr.time = `${Math.floor(timeInt / aggregatedTimeUnit)}`;
      }
      result.push(tr);
    }
  });

  return result;
}
