import plotly.io as pio
import plotly.graph_objects as go

pio.orca.config.use_xvfb = False

# Define NoC dimension
noc_dim = 4
annotation_font_size = 40 * (4 / noc_dim)

# Define colors
mvm_colors = [
  '#fc9849',
  '#ffd559',
  '#8ccc4b',
  '#3e6278',
  '#4789b2',
  '#99aeb4',
  '#88dfd7',
  '#fff9b6',
  '#ffbe97',
  '#f6779c',
  '#9f57af',
  '#29213b',
  '#363d69',
  '#7f4a3d'
]
dispatchers_color = '#d05757'
collector_color = '#3e9d4d'

# Parse placement
dispatchers = []
collector = 0
mvms = []
layer_mvms = []
current_layer = 0
place_file = open('mlp.place', 'r')
lines = place_file.readlines()
for line in lines:
  if 'layer' in line:
    line = line.replace('\n', '')
    l = line.split(' ')
    module = l[0].split('_')
    layer_num = int(module[0][5:])
    mvm_num = int(module[1][3:])
    location = int(l[2])
    if (layer_num > current_layer):
      mvms.append(layer_mvms)
      layer_mvms = []
      current_layer = current_layer + 1
    layer_mvms.append(location)
  elif 'input_dispatcher' in line:
    line = line.replace('\n','')
    l = line.split(' ')
    dispatchers.append(int(l[2]))
  elif 'output_collector' in line:
    line = line.replace('\n','')
    l = line.split(' ')
    collector = int(l[2])
mvms.append(layer_mvms)

scatter_plot = []

# Add dispatcher traces 
for d in range(len(dispatchers)):
    x_coordinate = dispatchers[d] % noc_dim
    y_coordinate = int(dispatchers[d] / noc_dim)
    scatter_plot.append(
      go.Scatter(x=[x_coordinate, x_coordinate, x_coordinate+1, x_coordinate+1, None], 
        y=[y_coordinate, y_coordinate+1, y_coordinate+1, y_coordinate, None], 
        fill='toself', fillcolor=dispatchers_color, line=dict(color=dispatchers_color, width=1), 
        mode='lines', showlegend=False, name='Dispatchers', text='trial', textposition='bottom right')
    )
    scatter_plot.append(
      go.Scatter(x=[x_coordinate + 0.5], y=[y_coordinate + 0.5], 
        fill='toself', fillcolor=dispatchers_color, line=dict(color=dispatchers_color, width=1), 
        mode='text', showlegend=False, name='Dispatchers', text='<b>D'+str(d)+'</b>', textposition='middle center')
    )

# Add collector traces
x_coordinate = collector % noc_dim
y_coordinate = int(collector / noc_dim)
scatter_plot.append(
  go.Scatter(x=[x_coordinate, x_coordinate, x_coordinate+1, x_coordinate+1, None], 
    y=[y_coordinate, y_coordinate+1, y_coordinate+1, y_coordinate, None], 
    fill='toself', fillcolor=collector_color, line=dict(color=collector_color, width=1), 
    mode='lines', showlegend=False, name='Collector')
)
scatter_plot.append(
  go.Scatter(x=[x_coordinate + 0.5], y=[y_coordinate + 0.5], 
    fill='toself', fillcolor=dispatchers_color, line=dict(color=dispatchers_color, width=1), 
    mode='text', showlegend=False, name='Dispatchers', text='<b>C</b>', textposition='middle center')
)

# Add MVM traces
for l in range(len(mvms)):
    for m in range(len(mvms[l])):
        x_coordinate = mvms[l][m] % noc_dim
        y_coordinate = int(mvms[l][m] / noc_dim)
        scatter_plot.append(
          go.Scatter(x=[x_coordinate, x_coordinate, x_coordinate+1, x_coordinate+1, None], 
          y=[y_coordinate, y_coordinate+1, y_coordinate+1, y_coordinate, None], 
          fill='toself', fillcolor=mvm_colors[l], line=dict(color=mvm_colors[l], width=1), 
          mode='lines', showlegend=False, name='Layer ' + str(l))
        )
        scatter_plot.append(
          go.Scatter(x=[x_coordinate + 0.5], y=[y_coordinate + 0.5], 
            fill='toself', fillcolor=dispatchers_color, line=dict(color=dispatchers_color, width=1), 
            mode='text', showlegend=False, name='Dispatchers', text='<b>L'+str(l)+'M'+str(m)+'</b>', textposition='middle center')
        )

fig = go.Figure(data=scatter_plot)

fig.update_traces(
    marker_size=3,
    marker_line=dict(width=0),
    textfont_size=annotation_font_size,
    textfont_color='black',
)

fig.update_xaxes(
    showline=True, 
    linewidth=2, 
    linecolor='black', 
    mirror=True,
    range=[0, noc_dim],
    showgrid=True,
    gridwidth=1,
    gridcolor='rgb(211,211,211)',
    tickfont=dict(family='Rockwell', color='white', size=1), 
    titlefont=dict(family='Rockwell', color='white', size=1),
    dtick=1
)

fig.update_yaxes(
    showline=True, 
    linewidth=2, 
    linecolor='black',
    mirror=True,
    range=[0, noc_dim], 
    tickfont=dict(family='Rockwell', color='white', size=1), 
    titlefont=dict(family='Rockwell', color='white', size=1),
    showgrid=True, 
    gridwidth=1, 
    gridcolor='rgb(211,211,211)',
    dtick=1
)

fig.update_layout(
    plot_bgcolor='white',
    height=800, 
    width=800,
    legend=dict(
        yanchor="bottom",
        y=1.01,
        xanchor="left",
        x=0.01,
        orientation="h",
        font=dict(family="Rockwell", size=20, color="black"),
        bgcolor="White",
        bordercolor="Black",
        borderwidth=2
    )
)

fig.update_traces(
    marker_line_color='black',
    marker_line_width=2
)

fig.write_image('noc.pdf')