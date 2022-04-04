import plotly.express as px
import math
from plotly.subplots import make_subplots
import plotly.graph_objects as go
import plotly.io as pio
import numpy as np
from numpy import loadtxt

pio.orca.config.use_xvfb = False

num_npu_sectors = 5
num_pipeline_blocks = 4
num_monitored_traces = 5

vspacing = 0.25
vspacing_step = vspacing / 5.0
box_voffset_up = 0.5 * vspacing_step
box_voffset_down = 1.5 * vspacing_step

uop_issue_trace_x = []
uop_issue_trace_y = []
uop_retire_trace_x = []
uop_retire_trace_y = []
tag_update_trace_x = []
tag_update_trace_y = []
boxes_x = []
boxes_y = []

trace_count = 0
module_count = 0
with open('npu_traces') as traces_file:
    for line in traces_file:
        # Extract integer values of a trace
        trace = line.strip().split(' ')
        if '' in trace:
          trace.remove('')
        trace = [int(i) for i in trace]
        trace_height = vspacing * ((num_npu_sectors * num_pipeline_blocks + 1) - module_count)
        # Append values to corresponding trace list
        if trace_count == 0:
          for i in trace:
            uop_issue_trace_x.append(i)
            uop_issue_trace_y.append(trace_height)
        elif trace_count == 1:
          for i in trace:
            uop_retire_trace_x.append(i)
            uop_retire_trace_y.append(trace_height - vspacing_step)
        elif trace_count == 2:
          next_trace = next(traces_file)
          next_trace = next_trace.strip().split(' ')
          if '' in next_trace:
            next_trace.remove('')
          next_trace = [int(i) for i in next_trace]
          for i, j in zip(trace, next_trace):
            boxes_x.append([i, j, j, i, i])
            boxes_y.append([trace_height-box_voffset_down, trace_height-box_voffset_down, trace_height+box_voffset_up, trace_height+box_voffset_up, trace_height-box_voffset_down])
          trace_count = trace_count + 1
        elif trace_count == 4:
          for i in trace:
            tag_update_trace_x.append(i)
            tag_update_trace_y.append(trace_height + vspacing_step)
        # Update trace and module counters
        if trace_count == num_monitored_traces - 1:
          trace_count = 0
          module_count = module_count + 1
        else:
          trace_count = trace_count + 1

fig = go.Figure()

fig.add_trace(go.Scatter(x=uop_issue_trace_x, y=uop_issue_trace_y, mode='markers', marker_color='green', marker_symbol='circle', marker_size=10, name='uOP Issue'))
fig.add_trace(go.Scatter(x=uop_retire_trace_x, y=uop_retire_trace_y, mode='markers', marker_color='red', marker_symbol='circle', marker_size=10, name='uOP Retire'))
fig.add_trace(go.Scatter(x=tag_update_trace_x, y=tag_update_trace_y, mode='markers', marker_color='blue', marker_symbol='star', marker_size=10, name='Tag Update', marker_line_color='black'))
for (i, j) in zip(boxes_x, boxes_y):
  fig.add_trace(go.Scatter(x=i, y=j, mode='lines', line_color='black', line_width=2, showlegend=False))

tick_vals = []
tick_text = []
for i in range(num_npu_sectors * num_pipeline_blocks + 1):
  tick_vals.append(vspacing * (i+1))
for i in range(num_npu_sectors):
  tick_text.append('MVU_' + str(i))
  tick_text.append('eVRF_' + str(i))
  tick_text.append('MFU0_' + str(i))
  tick_text.append('MFU1_' + str(i))
tick_text.append('Loader')
tick_vals.reverse()

fig.update_xaxes(showline=True, linewidth=2, linecolor='black', mirror=True)
fig.update_xaxes(showgrid=True, gridcolor='rgb(211,211,211)')
fig.update_xaxes(tickfont=dict(family='Arial', color='black', size=25), title="Simulation Cycles", titlefont=dict(family='Arial', color='black', size=25))
fig.update_xaxes(tick0=0, ticks="inside")
fig.update_yaxes(showline=True, linewidth=2, linecolor='black', mirror=True)
fig.update_yaxes(tickmode='array', tickvals=tick_vals, ticktext=tick_text, tickfont=dict(family='Arial', color='black', size=25))
fig.update_yaxes(showgrid=False)

fig.update_layout(xaxis_range=[0, uop_retire_trace_x[-1]+10])
fig.update_layout(plot_bgcolor='white')
fig.update_layout(
    legend=dict(
        font=dict(
            family="Arial",
            size=25,
            color="black"
        ),
        orientation="h",
        bgcolor="White",	
        bordercolor="Black",
        borderwidth=2,
        xanchor="left", x=0.01,
        yanchor="bottom", y=1.01
    )
)

fig.update_layout(height=(num_npu_sectors * num_pipeline_blocks + 1) * 70)

fig.show()

'''

fig.update_xaxes(showline=True, linewidth=2, linecolor='black', mirror=True)
#fig.update_xaxes(tickfont=dict(family='Rockwell', color='black', size=25), title="BRAM to LB Ratio", titlefont=dict(family='Rockwell', color='black', size=25))
fig.update_xaxes(tickfont=dict(family='Rockwell', color='black', size=25), title="Adder to LUT Ratio", titlefont=dict(family='Rockwell', color='black', size=25))
fig.update_yaxes(showline=True, linewidth=2, linecolor='black', mirror=True)
#fig.update_yaxes(title_text="DSP to LB Ratio", tickfont=dict(family='Rockwell', color='black', size=25), titlefont=dict(family='Rockwell', color='black', size=25))
fig.update_yaxes(title_text="FF to LUT Ratio", tickfont=dict(family='Rockwell', color='black', size=25), titlefont=dict(family='Rockwell', color='black', size=25))

fig.update_layout(plot_bgcolor='white')
fig.update_layout(height=800, width=)
fig.update_layout(
    legend=dict(
        font=dict(
            family="Rockwell",
            size=25,
            color="black"
        ),
        orientation="h",
        bgcolor="White",	
        bordercolor="Black",
        borderwidth=2,
        xanchor="left", x=0.01,
        yanchor="bottom", y=1.05
    )
)

fig.update_traces(marker_line_color='black', marker_line_width=2)
fig.update_xaxes(tick0=0, ticks="inside")
fig.update_yaxes(tick0=0, ticks="inside")
fig.update_yaxes(showgrid=True, gridwidth=1, gridcolor='rgb(211,211,211)')
fig.update_xaxes(showgrid=True, gridwidth=1, gridcolor='rgb(211,211,211)')

#fig.update_layout(margin=dict(l=200))

fig.write_image("scatter_2.pdf")
fig.show()'''