exe_cyc = 4
trace = [
    #
    # Stage 1: Scheduler broadcast instruction packets
    #
    ('X0Y0', 'X0Y1', 1, 6, 'inst'),  # Scheduler -> PE 1: five packets
    ('X0Y0', 'X1Y0', 1, 6, 'inst'),  #           -> PE 2: five packets
    ('X0Y0', 'X1Y0', 1, 6, 'inst'),  #           -> PE 3: five packets

    ('X0Y1', 'X0Y2', 2, 7, 'inst'),  # Relay (0,1) -> PE 1 (last relay)
    ('X1Y0', 'X1Y1', 2, 7, 'inst'),  # Relay (1,0) -> PE 2
    ('X1Y0', 'X2Y0', 2, 7, 'inst'),  # Relay (1,0) -> PE 3

    ('X1Y1', 'X1Y2', 3, 8, 'inst'),  # Relay (1,1) -> PE 2 (last relay)
    ('X2Y0', 'X2Y1', 3, 8, 'inst'),  # Relay (2,0) -> PE 3

    ('X2Y1', 'X2Y2', 4, 9, 'inst'),  # Relay (2,1) -> PE 3 (last relay)

    #
    # Stage 2: PE processing and access memory
    #

    # PE 1 waits for all five packets to be arrive, then starts processing
    ('X0Y2', 'X0Y1', 7, 9, 'mem_req'),   # PE 1  -> Mem 1: two mem requests (packets)
    ('X0Y1', 'X0Y2', 8, 10, 'mem_resp'), # Mem 1 -> PE 1 : two mem responses (packets)

    # PE 2 waits for all five packets to be arrive, then starts processing
    ('X1Y2', 'X1Y1', 8, 10, 'mem_req'),  # PE 2  -> Mem 2
    ('X1Y1', 'X1Y2', 9, 11, 'mem_resp'), # Mem 2 -> PE 2

    # PE 3 waits for all five packets to be arrive, then starts processing
    ('X2Y2', 'X2Y1', 9, 11, 'mem_req'),   # PE 3  -> Mem 3
    ('X2Y1', 'X2Y2', 10, 12, 'mem_resp'), # Mem 3 -> PE 3

    #
    # Stage 3: PE sends results back to scheduler
    #

    # PE 1 finishes after `exe_cyc` time
    ('X0Y2', 'X0Y1', 10+exe_cyc, 13+exe_cyc, 'exit'),  # PE 1        -> Relay (0,1)
    ('X0Y1', 'X0Y0', 11+exe_cyc, 14+exe_cyc, 'exit'),  # Relay (0,1) -> Scheduler

    # PE 2 finishes after `exe_cyc` time
    ('X1Y2', 'X1Y1', 11+exe_cyc, 14+exe_cyc, 'exit'),  # PE 2        -> Relay (1,1)
    ('X1Y1', 'X1Y0', 12+exe_cyc, 15+exe_cyc, 'exit'),  # Relay (1,1) -> Relay (1,0)
    ('X1Y0', 'X0Y0', 13+exe_cyc, 16+exe_cyc, 'exit'),  # Relay (1,0) -> Scheduler

    # PE 3 finishes after `exe_cyc` time
    ('X2Y2', 'X2Y1', 12+exe_cyc, 15+exe_cyc, 'exit'),  # PE 3        -> Relay (2,1)
    ('X2Y1', 'X2Y0', 13+exe_cyc, 16+exe_cyc, 'exit'),  # Relay (2,1) -> Relay (2,0)
    ('X2Y0', 'X1Y0', 14+exe_cyc, 17+exe_cyc, 'exit'),  # Relay (2,0) -> Relay (1,0)
    ('X1Y0', 'X0Y0', 15+exe_cyc, 18+exe_cyc, 'exit'),  # Relay (2,1) -> Scheduler
]

with open('./input/traces.csv', 'w') as f:
    f.write('time, pkt_src, pkt_dst, pkt_type')
    f.write('\n')
    for x in trace:
        src, dst, start_time, cease_time, pkt_type = x
        for time in range(start_time, cease_time):
            f.write(f'{time}, {src}, {dst}, {pkt_type}')
            f.write('\n')
