<script setup lang="ts">
import {
    FwbCheckbox,
    FwbAccordion,
    FwbAccordionContent,
    FwbAccordionHeader,
    FwbAccordionPanel,
    FwbTextarea,
    FwbButton,
    FwbSelect,
    FwbRange,
    FwbInput,
    FwbModal
} from 'flowbite-vue'

import { ref, computed, watch, reactive } from 'vue';

//
// Control
//

interface Props {
    open: boolean, // edge triggering (both pos and neg)
    maxTimeUnit: number,
};
const props = defineProps<Props>()
const emit = defineEmits(['changedTraceInput']);

const openTrigger = computed(() => props.open);
const maxTimeUnit = computed(() => props.maxTimeUnit);

watch(openTrigger, () => {
    isShowTraceModal.value = true;
})

const isShowTraceModal = ref(false)

function closeTraceModal() {
    isShowTraceModal.value = false
}

//
// Data
//

import { TracePanelInput } from '@/data';

const traceFormatOptions = [
    { value: 'rad-sim', name: 'RAD-Sim Flit Trace Format' },
    { value: 'low-level', name: 'Assembly-like Low-level Trace Format' }
];

const traceAggregationMethodOptions = [
    { value: 'avg', name: 'Average the number of flits (for link weight aggregation in the Spatial View)' },
    { value: 'sum', name: 'Sum the number of flits (for link weight aggregation in the Spatial View)' }
]

const input = reactive<TracePanelInput>({ fmt: '', trace_flits: '', trace_stats: '', unit: 50, aggregation: '', sum_aggregation_link_capacity: 100 });
const sumAggLinkCapString = ref('');
const isTraceFlitsInputSaved = ref(false);
const isTraceStatsInputSaved = ref(false);
const isOptionalTracesEnabled = ref(false);
const isSumAggregationLinkCapSaved = ref(false);

import { traceFlitsExampleCSVString, traceStatsExampleCSVString } from '@/input-loader'

function useTraceExample() {
    input.fmt = 'rad-sim';
    input.trace_flits = traceFlitsExampleCSVString;
    isOptionalTracesEnabled.value = true;
    input.trace_stats = traceStatsExampleCSVString;
    // TODO: considering the following watchers, do we need to add `input.unit`?
    input.aggregation = 'avg';
}

function saveTraceFlitsInput() {
    isTraceFlitsInputSaved.value = true;
    emit('changedTraceInput', input);
}

function saveTraceStatsInput() {
    isTraceStatsInputSaved.value = true;
    emit('changedTraceInput', input);
}

function saveSumAggregationLinkCap() {
    isSumAggregationLinkCapSaved.value = true;
    input.sum_aggregation_link_capacity = Number(sumAggLinkCapString.value);
    emit('changedTraceInput', input);
}

watch([() => input.fmt, () => input.trace_flits, () => input.trace_stats], () => {
    isTraceFlitsInputSaved.value = false;
});

watch([() => input.unit, () => input.aggregation], () => {
    emit('changedTraceInput', input);
});

watch(sumAggLinkCapString, () => {
    isSumAggregationLinkCapSaved.value = false;
});

watch(isOptionalTracesEnabled, (enabled: boolean) => {
    if (!enabled) {
        input.trace_stats = '';
    }
})

// Use example by default
// TODO: make it more maintainable
emit('changedTraceInput', { fmt: 'rad-sim', trace_flits: traceFlitsExampleCSVString, trace_stats: traceStatsExampleCSVString, unit: 50, aggregation: 'avg', sum_aggregation_link_capacity: 100 });

</script>

<template>
    <fwb-modal v-if="isShowTraceModal" @close="closeTraceModal">
        <template #header>
            <div class="flex items-center text-lg">
                Traces from the Simulation
            </div>
        </template>
        <template #body>

            <!-- Body of the modal start -->

            <p class="mb-3 text-base leading-relaxed" style="padding-bottom: 0.4rem;">
                <span class="block mb-2 text-base font-semibold text-gray-900 dark:text-white">Example:</span>
                <fwb-button color="default" @click="useTraceExample">MLP (Mesh NoC 4x4)</fwb-button>
            </p>

            <div style="margin-left: -1.25rem; margin-right: -1.25rem;">
                <!-- the body of the modal uses TailWind `p-6` padding (1.5rem), and
                    the text of the accordion header uses `p-5` heading (1.25rem) -->
                <fwb-accordion flush always-open>
                    <fwb-accordion-panel>
                        <fwb-accordion-header>Input Flit-based Traces</fwb-accordion-header>
                        <fwb-accordion-content>
                            <div>
                                <p class="mb-3 text-base leading-relaxed">
                                    <fwb-select v-model="input.fmt" :options="traceFormatOptions"
                                        label="Select the flit trace format:" />
                                    <fwb-textarea v-model="input.trace_flits" :rows="15" label=""
                                        placeholder="Paste your flit traces ..." />
                                </p>
                                <p class="block mt-2 mb-3">
                                    <fwb-button @click="saveTraceFlitsInput" color="green"
                                        :outline="isTraceFlitsInputSaved">
                                        <template v-if="isTraceFlitsInputSaved"> Saved </template>
                                        <template v-else> Save Traces </template>
                                    </fwb-button>
                                </p>
                            </div>
                        </fwb-accordion-content>
                    </fwb-accordion-panel>
                    <fwb-accordion-panel>
                        <fwb-accordion-header>Input Optional Traces</fwb-accordion-header>
                        <fwb-accordion-content>
                            <div>
                                <p class="block mt-2 mb-3">
                                    <fwb-checkbox v-model="isOptionalTracesEnabled"
                                        label="Enable optional (supplementary) traces feature" />
                                </p>
                                <template v-if="isOptionalTracesEnabled">
                                    <p class="mb-3 text-base leading-relaxed">
                                        <fwb-textarea v-model="input.trace_stats" :rows="15" label=""
                                            placeholder="Paste your optional traces ..." />
                                    </p>
                                    <p class="block mt-2 mb-3">
                                        <fwb-button @click="saveTraceStatsInput" color="green"
                                            :outline="isTraceStatsInputSaved">
                                            <template v-if="isTraceStatsInputSaved"> Saved </template>
                                            <template v-else> Save Traces </template>
                                        </fwb-button>
                                    </p>
                                </template>

                                <p class="mb-2 text-gray-500 dark:text-gray-400">
                                    Currently, the <i>optional (supplementary) trace</i> feature is exclusively available
                                    for RAD-Sim, enabling the identification of the source and destination modules (or NoC
                                    routers) of a flit's transaction. In the future, this feature might be expanded to
                                    accommodate a broader range of purposes.
                                </p>
                            </div>
                        </fwb-accordion-content>
                    </fwb-accordion-panel>
                    <fwb-accordion-panel>
                        <fwb-accordion-header>Switch Trace Aggregation Behaviors</fwb-accordion-header>
                        <fwb-accordion-content>
                            <div>
                                <div class="mb-2 text-base leading-relaxed">
                                    <span class="block mb-2 text-base font-semibold text-gray-900 dark:text-white">Aggregate
                                        traces in the time
                                        unit of {{ input.unit }} ns:</span>
                                    <p class="mt-2 mb-4">
                                        <fwb-range v-model="input.unit" :min="1" :max="maxTimeUnit" label="Time Unit (ns)"
                                            :disabled="!isTraceFlitsInputSaved" />
                                    </p>
                                    <p class="mt-2">
                                        <fwb-select v-model="input.aggregation" :options="traceAggregationMethodOptions"
                                            label="Select the link weight aggregation method:" />
                                    </p>
                                    <template v-if="input.aggregation == 'sum'">
                                        <p class="mt-4">
                                            <fwb-input v-model="sumAggLinkCapString"
                                                label="Enter the link capacity upper bound for sum-mode aggregation:"
                                                placeholder="Enter a positive integer ..." size="lg">
                                                <!-- <template #prefix>
                                                TODO: use number svg
                                                </template> -->
                                                <template #suffix>
                                                    <fwb-button @click="saveSumAggregationLinkCap" color="green"
                                                        :outline="isSumAggregationLinkCapSaved">
                                                        <template v-if="isSumAggregationLinkCapSaved"> Saved </template>
                                                        <template v-else> Save </template>
                                                    </fwb-button>
                                                </template>
                                            </fwb-input>
                                        </p>
                                    </template>
                                </div>
                            </div>
                        </fwb-accordion-content>
                    </fwb-accordion-panel>
                </fwb-accordion>
            </div>

            <!-- Body of the modal ends -->

        </template>
        <template #footer>
            <p class="text-base leading-relaxed text-gray-500 dark:text-gray-400">
                Please (1) input the traces first and then (2) set the trace aggregation mode and time granularity.
            </p>
        </template>
    </fwb-modal>
</template>
