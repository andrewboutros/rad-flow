import plotly.express as px
import math
from plotly.subplots import make_subplots
import plotly.graph_objects as go
import plotly.io as pio
import numpy as np
from numpy import loadtxt

pio.orca.config.use_xvfb = False

num_traces = 8

vspacing = 0.25
vspacing_step = vspacing / 5.0

traces_names = [
    "Embed. LU Req",
    "Mem0 Resp",
    "Mem1 Resp",
    "Mem2 Resp",
    "Mem3 Resp",
    "Feat. Inter. Out",
    "MLP Start",
    "MLP End",
]
traces_color = [
    "#003f5c",
    "#2f4b7c",
    "#665191",
    "#a05195",
    "#d45087",
    "#f95d6a",
    "#ff7c43",
    "#ffa600",
]

traces_x = []
traces_y = []
for i in range(num_traces):
    traces_x.append([])
    traces_y.append([])

trace_count = 0
max_val = 0
with open("../../../sim/sim.trace") as traces_file:
    for line in traces_file:
        # Extract integer values of a trace
        trace = line.strip().split(" ")
        if "" in trace:
            trace.remove("")
        trace = [int(i) for i in trace]
        trace_height = vspacing * ((num_traces - trace_count + 1))

        for i in trace:
            # Append values to corresponding trace list
            traces_x[trace_count].append(i)
            traces_y[trace_count].append(trace_height)
            if i > max_val:
                max_val = i

        # Update trace counter
        if trace_count == num_traces - 1:
            trace_count = 0
        else:
            trace_count = trace_count + 1

fig = go.Figure()

for i in range(len(traces_x)):
    fig.add_trace(
        go.Scatter(
            x=traces_x[i],
            y=traces_y[i],
            mode="markers",
            marker_symbol="circle",
            marker_color=traces_color[i],
            marker_size=10,
        ),
    )

tick_vals = []
tick_text = traces_names
for i in range(num_traces + 1):
    tick_vals.append(vspacing * (i + 1))
tick_vals.reverse()

fig.update_xaxes(showline=True, linewidth=2, linecolor="black", mirror=True)
fig.update_xaxes(showgrid=True, gridcolor="rgb(211,211,211)")
fig.update_xaxes(
    tickfont=dict(family="Arial", color="black", size=25),
    title="Simulation Cycles",
    titlefont=dict(family="Arial", color="black", size=25),
)
fig.update_xaxes(tick0=0, ticks="inside")
fig.update_yaxes(showline=True, linewidth=2, linecolor="black", mirror=True)
fig.update_yaxes(
    tickmode="array",
    tickvals=tick_vals,
    ticktext=tick_text,
    tickfont=dict(family="Arial", color="black", size=25),
)
fig.update_yaxes(showgrid=False)

#fig.update_layout(xaxis_range=[0, max_val + 10])
fig.update_layout(xaxis_range=[0, 5000 + 10])
fig.update_layout(plot_bgcolor="white")
fig.update_layout(showlegend=False)

fig.update_layout(height=num_traces * 55)

fig.show()
