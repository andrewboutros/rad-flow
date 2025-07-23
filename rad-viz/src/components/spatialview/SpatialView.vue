<script setup lang="ts">
import { ref, shallowRef, watch, computed, onMounted } from "vue";

import { SpatialViewRendering, SpatialViewModuleNoCPlacement, OptionalTrace, RADSimTraceStatsFormat } from "@/data";

import {
  base64ToSvgString,
  stringToSvg,
  decodeDiagramXmlString,
  stripXmlString,
  getContrastFontColor,
} from "@/utils";

import DiagramModel from './model/diagram';

import SpatialViewModel from './model/spatialview';

import TransformDiagramModelToSpatialViewModel from './model/transform';

import { EditorBus } from "./diagrams";

import FlylineIndicator from "./model/addons/flylineindicator";

import Papa from "papaparse";

import * as d3 from "d3";

//
// Properties
//

interface Props {
  diagram: string;
  data: SpatialViewRendering[];
  placement: SpatialViewModuleNoCPlacement[];
  optional: string;
  legendScale: string;
  edit: boolean; // edge triggering (both pos and neg)
  refresh: boolean; // edge triggering (both pos and neg)
}

const props = defineProps<Props>();

const renderData = computed(() => props.data); // spatial view (sv) model only
const optionalTraceCSVString = computed(() => props.optional); // spatial view (sv) model only
const diagramXMLString = computed(() => props.diagram); // diagram model --> sv model
const modulePlacementData = computed(() => props.placement); // diagram model --> sv model
const legendScaleData = computed(() => props.legendScale); // legend (partially sv model)
const editTrigger = computed(() => props.edit); // diagram model --> sv model
const refreshTrigger = computed(() => props.refresh); // diagram model --> sv model

//
// Local Variables
//

const divRef = ref(null);
const divLegendRef = ref(null);
const diagramModel = shallowRef<DiagramModel>();
const spatialViewModel = shallowRef<SpatialViewModel>();

//
// Watchers
//

// TODO: add store option in arch. modal which reflects the modified arch.
// TODO: remove refreshing logic
watch([diagramXMLString, modulePlacementData, refreshTrigger], () => {
  spatialViewModel.value = generateNewSpatialViewModel(diagramXMLString.value, modulePlacementData.value);
  spatialViewModel.value.registerAddon('flyline', new FlylineIndicator(modulePlacementData.value, parseOptionalTraceCSVString(optionalTraceCSVString.value)));
  // TODO: double check
  feedRenderDataToSpatialViewModel(renderData.value);
  renderSpatialViewLegend(legendScaleData.value);
});

watch(editTrigger, () => edit());

watch(renderData, () => {
  feedRenderDataToSpatialViewModel(renderData.value);
});

watch(legendScaleData, (scale) => {
  renderSpatialViewLegend(scale);
});

function parseOptionalTraceCSVString(csv: string) {
  // TODO: support multiple and different optional traces for others purposes
  const data = Papa.parse(csv, {
    header: true,
    skipEmptyLines: true,
    transformHeader: (header: string) => header.trim(),
  }).data as Array<RADSimTraceStatsFormat>;
  const result = new Array<OptionalTrace>();
  data.forEach((d) => {
    result.push({
      id: d.id.trim(),
      src: d.src.trim(),
      dst: d.dest.trim(),
    });
  });
  return result;
}

watch(optionalTraceCSVString, (csv) => {
  if (spatialViewModel.value !== undefined) {
    spatialViewModel.value.registerAddon('flyline', new FlylineIndicator(modulePlacementData.value, parseOptionalTraceCSVString(csv)));
  }
});


function edit() {
  if (diagramModel.value !== undefined) {
    const bus = new EditorBus({
      data: diagramModel.value.XMLStringTemplateForEditing(),
      format: "xmlsvg",
      onExport: (svg: string) => {
        const svgStr = base64ToSvgString(svg);
        if (svgStr) {
          const diagramXML = stringToSvg(svgStr)!.getAttribute("content")!;
          // TODO: consider combining diagramXML with the one in the props
          spatialViewModel.value = generateNewSpatialViewModel(decodeDiagramXmlString(diagramXML), modulePlacementData.value);
          spatialViewModel.value.registerAddon('flyline', new FlylineIndicator(modulePlacementData.value, parseOptionalTraceCSVString(optionalTraceCSVString.value)));
          feedRenderDataToSpatialViewModel(renderData.value);
          renderSpatialViewLegend(legendScaleData.value);
        }
      },
    });
    bus.startEdit();
  }
  // TODO: corner-case warning
}


function generateNewSpatialViewModel(diagram: string, placement: SpatialViewModuleNoCPlacement[]) {
  diagramModel.value = new DiagramModel(stripXmlString(diagram));
  // TODO: capsulize vertex effect logics
  const moduleGroupNameSet = new Set<string>();
  placement.forEach((vertex) => {
    moduleGroupNameSet.add(vertex.group_name);
  });
  const moduleGroupNameColorMapping = new Map<string, string>();
  Array.from(moduleGroupNameSet).sort().forEach((group_name, i) => {
    const x = 1.0 / moduleGroupNameSet.size;
    moduleGroupNameColorMapping.set(group_name, d3.color(d3.interpolateGreys(i * x))!.formatHex())
  })
  diagramModel.value!.setAllVertexStyle("strokeColor", "#000000")
  placement.every((vertex) => {
    const vertexFillColor = moduleGroupNameColorMapping.get(vertex.group_name)!;
    const vertexStrokeColor = d3.color(vertexFillColor)?.darker().formatHex()!;
    const vertexFontColor = getContrastFontColor(vertexFillColor);
    let valid = true;
    valid = valid && diagramModel.value!.setVertexText(vertex.node_id, vertex.module_name);
    valid = valid && diagramModel.value!.setVertexStyle(vertex.node_id, "fillColor", vertexFillColor);
    valid = valid && diagramModel.value!.setVertexStyle(vertex.node_id, "strokeColor", vertexStrokeColor);
    valid = valid && diagramModel.value!.setVertexStyle(vertex.node_id, "fontColor", vertexFontColor);
    if (!valid) {
      alert(`Node ID ${vertex.node_id} in the placement file is invalid`);
    }
    return valid;
  });
  return TransformDiagramModelToSpatialViewModel(diagramModel.value);
}

function feedRenderDataToSpatialViewModel(data: SpatialViewRendering[]) {
  const svg = spatialViewModel.value!.feed(data);
  svg && (divRef.value! as HTMLDivElement).replaceChildren(svg);
}

function renderSpatialViewLegend(legendScaleString: string) {
  const svg = spatialViewModel.value!.legend(legendScaleString);
  svg && (divLegendRef.value! as HTMLDivElement).replaceChildren(svg);
}

onMounted(() => {
  spatialViewModel.value = generateNewSpatialViewModel(diagramXMLString.value, modulePlacementData.value);
  spatialViewModel.value.registerAddon('flyline', new FlylineIndicator(modulePlacementData.value, parseOptionalTraceCSVString(optionalTraceCSVString.value)));
  feedRenderDataToSpatialViewModel(renderData.value);
  renderSpatialViewLegend(legendScaleData.value);
});

</script>

<template>
  <div id="spatial-view-legend" ref="divLegendRef"></div>
  <div id="spatial-view-body" ref="divRef"></div>
</template>

<style scoped>
#spatial-view-body {
  display: flex;
  width: 100vw;
  justify-content: center;
}

#spatial-view-legend {
  position: absolute;
  top: inherit;
  left: 3.5em;
  width: 5.5em;
  overflow: visible;
  user-select: none;
}
</style>
