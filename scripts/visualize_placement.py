import plotly.io as pio
import plotly.graph_objects as go

pio.orca.config.use_xvfb = False

tick_font_size = 20
title_font_size = 20
legend_font_size = 20

noc_dim = 4
annotation_font_size = 40 * (4 / noc_dim)

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

dispatchers = [0, 5, 7]
collector = 9
mvms = [[1, 6], [10, 13, 2], [4]]

scatter_plot = []

for d in range(len(dispatchers)):
    x_coordinate = dispatchers[d] % noc_dim
    y_coordinate = int(dispatchers[d] / noc_dim)
    scatter_plot.append(
      go.Scatter(x=[x_coordinate, x_coordinate, x_coordinate+1, x_coordinate+1, None], 
        y=[y_coordinate, y_coordinate+1, y_coordinate+1, y_coordinate, None], 
        fill='toself', fillcolor=dispatchers_color, line=dict(color=dispatchers_color, width=1), 
        mode='lines', showlegend=(d == 0), name='Dispatchers', text='trial', textposition='bottom right')
    )
    scatter_plot.append(
      go.Scatter(x=[x_coordinate + 0.5], y=[y_coordinate + 0.5], 
        fill='toself', fillcolor=dispatchers_color, line=dict(color=dispatchers_color, width=1), 
        mode='text', showlegend=(d == 0), name='Dispatchers', text='<b>D'+str(d)+'</b>', textposition='middle center')
    )

x_coordinate = collector % noc_dim
y_coordinate = int(collector / noc_dim)
scatter_plot.append(
  go.Scatter(x=[x_coordinate, x_coordinate, x_coordinate+1, x_coordinate+1, None], 
    y=[y_coordinate, y_coordinate+1, y_coordinate+1, y_coordinate, None], 
    fill='toself', fillcolor=collector_color, line=dict(color=collector_color, width=1), 
    mode='lines', showlegend=True, name='Collector')
)
scatter_plot.append(
  go.Scatter(x=[x_coordinate + 0.5], y=[y_coordinate + 0.5], 
    fill='toself', fillcolor=dispatchers_color, line=dict(color=dispatchers_color, width=1), 
    mode='text', showlegend=(d == 0), name='Dispatchers', text='<b>C</b>', textposition='middle center')
)

for l in range(len(mvms)):
    for m in range(len(mvms[l])):
        x_coordinate = mvms[l][m] % noc_dim
        y_coordinate = int(mvms[l][m] / noc_dim)
        scatter_plot.append(
          go.Scatter(x=[x_coordinate, x_coordinate, x_coordinate+1, x_coordinate+1, None], 
          y=[y_coordinate, y_coordinate+1, y_coordinate+1, y_coordinate, None], 
          fill='toself', fillcolor=mvm_colors[l], line=dict(color=mvm_colors[l], width=1), 
          mode='lines', showlegend=(m == 0), name='Layer ' + str(l))
        )
        scatter_plot.append(
          go.Scatter(x=[x_coordinate + 0.5], y=[y_coordinate + 0.5], 
            fill='toself', fillcolor=dispatchers_color, line=dict(color=dispatchers_color, width=1), 
            mode='text', showlegend=(d == 0), name='Dispatchers', text='<b>L'+str(l)+'M'+str(m)+'</b>', textposition='middle center')
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
        font=dict(family="Rockwell", size=legend_font_size, color="black"),
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