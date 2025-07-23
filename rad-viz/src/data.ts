export interface TemporalViewRendering {
  time: string; // time
  pkt_type: string; // pkt type
  pkt_count: number; // count of the certain pkt type during this time period
}

export interface SpatialViewRendering {
  src: string;
  dst: string;
  color: string; // related to `norm` field
  norm: number; // mapped from weight to [0..8]
  weight: number;
  groups: Array<string>;
  // width: string;
}

export interface SpatialViewModuleNoCPlacement {
  node_id: string;
  module_name: string;
  group_name: string;
}

export interface TracePanelInput {
  fmt: string;
  trace_flits: string;
  trace_stats: string;
  unit: number;
  aggregation: string;
  sum_aggregation_link_capacity: number;
}

export interface OptionalTrace {
  id: string; // the trans. id in the `groups` array of `SpatialViewRendering`
  src: string;
  dst: string;
}

//
// RAD-Sim exclusive
//

export interface RADSimTraceStatsFormat {
  id: string;
  noc_id: string;
  src: string;
  dest: string;
  dataw: string;
  hops: string;
  t_init: string;
  t_packet: string;
  t_inject: string;
  t_eject: string;
  t_depacket: string;
  t_receive: string;
  latency: string;
}
