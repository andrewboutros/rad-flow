import csv

quant = 100

print('time, pkt_src, pkt_dst, pkt_type')

# TODO: use corresponding placement file for router ID mapping instead of hardcodes
placement = ['L1M0', 'D3', 'D0', 'L0M2',
             'L0M0', 'C', 'L1M1', 'L1M2',
             'L3M0', 'L2M1', 'D2', 'L0M3',
             'L3M1', 'D1', 'L0M1', 'L2M0']

with open('flit_traces.csv', 'r') as f:
    reader = csv.reader(f)
    rows = list(reader)[1:]
    rows = sorted(rows, key=lambda x: int(x[0]))
    for row in rows:
        t_trace, src_router, dst_router, _, flit_type, _ = tuple(row)
        if int(src_router) == -1 or int(dst_router) == -1: # injection or ejection
            continue
        t_trace = int(int(t_trace) // quant)
        src_router = placement[int(src_router)]
        dst_router = placement[int(dst_router)]
        flit_type = str(flit_type).strip()
        print(f'{t_trace}, {src_router}, {dst_router}, {flit_type}')
