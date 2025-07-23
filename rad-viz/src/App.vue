<script setup lang="ts">
import {
    FwbNavbar,
    FwbNavbarCollapse,
    FwbNavbarLink,
} from 'flowbite-vue'

import { onMounted, ref, shallowRef } from 'vue';

//
// NavBar Controller
//

// NoC Arch Panel

import NoCArchPanel from '@/components/controlpanel/NoCArchPanel.vue';
import { SpatialViewModuleNoCPlacement } from '@/data';

const showNoCArchPanel = ref(false)
const spatialViewDiagram = shallowRef("") // together with refresh trigger
const spatialViewEditTrigger = ref(false)
const spatialViewRefreshTrigger = ref(false)
const spatialViewPlacementData = ref<SpatialViewModuleNoCPlacement[]>([]);

function NoCArchDiagramChanged(d: string) {
    spatialViewDiagram.value = d;
    // Hard refreshing is needed, since user might want to **reset** the diagram
    // after editing, but that manually edited diagram is not reflected in the
    // `spatialViewDiagram.value`
    spatialViewRefreshTrigger.value = !spatialViewRefreshTrigger.value;
}

function moduleNoCPlacementChanged(d: SpatialViewModuleNoCPlacement[]) {
    spatialViewPlacementData.value = d;
}

// Trace Panel

import TracePanel from '@/components/controlpanel/TracePanel.vue';
import { TracePanelInput } from '@/data';

const showTracePanel = ref(false)

function traceInputChanged(tr: TracePanelInput) {
    preprocessing.value = new Preprocessing(TransformIntoLowLevelTraceFormat(tr.fmt, tr.trace_flits, tr.unit));
    temporalViewData.value = preprocessing.value.getTemporalViewRenderingJson();
    temporalViewXAxisTimeUnit.value = tr.unit;
    spatialViewOptionalTraceCSVString.value = tr.trace_stats;
    spatialViewLinkAggregationMethod.value = tr.aggregation;
    spatialViewLinkAggregationSumLinkCapacityUpperBound.value = tr.sum_aggregation_link_capacity;
    if (temporalViewTimeRange.value !== undefined) {
        timeRangeChanged(temporalViewTimeRange.value); // re-calculate the spatial view data
    }
}

//
// Body Controller
//

import { Splitpanes, Pane } from 'splitpanes'
import 'splitpanes/dist/splitpanes.css'

import SpatialView from '@/components/spatialview/SpatialView.vue'
import TemporalView from '@/components/temporalview/TemporalView.vue'

import Preprocessing from '@/preprocessing/preprocessing'
import TransformIntoLowLevelTraceFormat from '@/preprocessing/traceformats/traceformats';
import { SpatialViewRendering, TemporalViewRendering } from '@/data'

const preprocessing = shallowRef<Preprocessing>();

const spatialViewData = ref<SpatialViewRendering[]>([]);
const spatialViewOptionalTraceCSVString = ref(''); // TODO: add a special module to process and categorize optional traces
const spatialViewLinkAggregationMethod = ref('');
const spatialViewLinkAggregationSumLinkCapacityUpperBound = ref(100);
const spatialViewLegendScale = ref(''); // TODO: make it more maintainable
const temporalViewData = ref<TemporalViewRendering[]>([]);
const temporalViewRefreshTrigger = ref(false) // TODO: tune the temporal view refreshing logic
const temporalViewPaneProportion = ref(25)
const temporalViewTimeRange = ref<[number, number]>();
const temporalViewXAxisTimeUnit = ref(1);

function timeRangeChanged(timeRange: [number, number]) {
    const [from, to] = timeRange;
    temporalViewTimeRange.value = timeRange;
    if (preprocessing.value !== undefined) {
        spatialViewLegendScale.value = (spatialViewLinkAggregationMethod.value == 'avg') ?
            ` ${(to - from) * temporalViewXAxisTimeUnit.value * 1 /* link capacity */ / 9}` :
            `^${spatialViewLinkAggregationSumLinkCapacityUpperBound.value / 9}`;
        spatialViewData.value = preprocessing.value.getRenderingJsonAtTimeRange(from, to, (val) => {
            if (spatialViewLinkAggregationMethod.value == 'avg') {
                return Math.ceil((val * 9) / ((to - from) * temporalViewXAxisTimeUnit.value * 1 /* link capacity */)) - 1;
            } else {
                const mappedValue = (Math.ceil((val * 9) / spatialViewLinkAggregationSumLinkCapacityUpperBound.value)) - 1;
                return mappedValue > 8 ? 8 : mappedValue;
            }
        });
    }
}

//
// Global Event Controller
//

const splitPanesHeight = ref(0);

onMounted(() => {
    splitPanesHeight.value = document.getElementById("split-panes-div")!.clientHeight;
    window.addEventListener("resize", (_) => {
        spatialViewRefreshTrigger.value = !spatialViewRefreshTrigger.value;
        temporalViewRefreshTrigger.value = !temporalViewRefreshTrigger.value;
        splitPanesHeight.value = document.getElementById("split-panes-div")!.clientHeight;
    });
});

</script>

<template>
    <div id="navbar-div">
        <fwb-navbar solid>
            <template #logo>
                <span class="text-lg font-medium">RAD-Viz</span>
            </template>
            <template #default="{ isShowMenu }">
                <fwb-navbar-collapse :is-show-menu="isShowMenu">
                    <fwb-navbar-link @click="showNoCArchPanel = !showNoCArchPanel" link="#">
                        NoC Architecture
                    </fwb-navbar-link>
                    <fwb-navbar-link @click="showTracePanel = !showTracePanel" link="#">
                        Traces
                    </fwb-navbar-link>
                    <fwb-navbar-link>
                        <a href="https://forms.gle/2gSf3u652FSwcUHcA" target="_blank" rel="noopener noreferrer">Feedback</a>
                    </fwb-navbar-link>
                </fwb-navbar-collapse>
            </template>
        </fwb-navbar>
        <NoCArchPanel @changed-arch-diagram="(d) => NoCArchDiagramChanged(d)"
            @edit-arch-diagram="spatialViewEditTrigger = !spatialViewEditTrigger"
            @changed-placement="(d) => moduleNoCPlacementChanged(d)" :open="showNoCArchPanel"></NoCArchPanel>
        <TracePanel @changed-trace-input="(tr) => traceInputChanged(tr)" :open="showTracePanel" :max-time-unit="100">
        </TracePanel>
    </div>
    <div id="split-panes-div">
        <splitpanes class="default-theme" horizontal @resize="temporalViewPaneProportion = $event[1].size"
            :dbl-click-splitter="false">
            <pane>
                <SpatialView :diagram="spatialViewDiagram" :data="spatialViewData"
                    :optional="spatialViewOptionalTraceCSVString" :placement="spatialViewPlacementData"
                    :edit="spatialViewEditTrigger" :refresh="spatialViewRefreshTrigger"
                    :legend-scale="spatialViewLegendScale"></SpatialView>
            </pane>
            <pane min-size="20" :size="temporalViewPaneProportion">
                <TemporalView :time-range="[0, 1]" :data="temporalViewData" :refresh="temporalViewRefreshTrigger"
                    :height="temporalViewPaneProportion * splitPanesHeight / 100" :time-unit="temporalViewXAxisTimeUnit"
                    @time-range-changed="timeRangeChanged">
                </TemporalView>
            </pane>
        </splitpanes>
    </div>
</template>

<style scoped>
#split-panes-div {
    height: calc(100vh - 76px);
}
</style>
