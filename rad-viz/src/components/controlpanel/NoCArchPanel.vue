<script setup lang="ts">
import {
    FwbTextarea,
    FwbButton,
    FwbModal
} from 'flowbite-vue'

import { ref, computed, watch } from 'vue';

//
// Control
//

interface Props {
    open: boolean, // edge triggering (both pos and neg)
};
const props = defineProps<Props>()
const emit = defineEmits(['editArchDiagram', 'changedArchDiagram', 'changedPlacement']);

const openTrigger = computed(() => props.open);

watch(openTrigger, () => {
    isShowNoCArchModal.value = true;
})

const isShowNoCArchModal = ref(false);

function closeNoCArchModal() {
    isShowNoCArchModal.value = false;
}

//
// Data
//

import { diagramExampleXMLString, placementExampleCSVString } from '@/input-loader'

const moduleNoCPlacementString = ref("");
const isPlacementSaved = ref(false);

function useMLPExample() {
    // NoC architecture diagram
    emit('changedArchDiagram', diagramExampleXMLString);
    // Module NoC placement
    moduleNoCPlacementString.value = placementExampleCSVString;
}

function editNoCArch() {
    emit('editArchDiagram');
}

import Papa from "papaparse";
import { SpatialViewModuleNoCPlacement } from '@/data';

function parsePlacementFile(csvString: string) {
    const data = Papa.parse(csvString, {
        header: true,
        skipEmptyLines: true,
        transformHeader: (header: string) => header.trim(),
    }).data as Array<SpatialViewModuleNoCPlacement>;

    data.forEach((d) => {
        d.node_id = d.node_id.trim();
        d.module_name = d.module_name.trim();
        d.group_name = d.group_name.trim();
    });

    return data;
}

function savePlacement() {
    isPlacementSaved.value = true;
    emit('changedPlacement', parsePlacementFile(moduleNoCPlacementString.value));
}

watch(moduleNoCPlacementString, () => {
    isPlacementSaved.value = false;
});

// Use example by default
// TODO: make it more maintainable
emit('changedArchDiagram', diagramExampleXMLString);
emit('changedPlacement', parsePlacementFile(placementExampleCSVString));

</script>

<template>
    <fwb-modal v-if="isShowNoCArchModal" @close="closeNoCArchModal">
        <template #header>
            <div class="flex items-center text-lg">
                NoC Architecture Specification
            </div>
        </template>
        <template #body>
            <p class="mb-3 text-base leading-relaxed">
                <span class="block mb-2 text-sm font-medium text-gray-900 dark:text-white">
                    Example:
                </span>
                <fwb-button color="default" @click="useMLPExample">MLP (Mesh NoC 4x4)</fwb-button>
            </p>

            <p class="mb-3 text-base leading-relaxed">
                <span class="block mb-2 text-sm font-medium text-gray-900 dark:text-white">
                    Edit the architecture manually:
                </span>
                <fwb-button color="default" @click="editNoCArch">Edit Architecture</fwb-button>
            </p>

            <fwb-textarea v-model="moduleNoCPlacementString" :rows="15" label="Module NoC Placement:"
                placeholder="Paste your placement file ..." />

            <p class="block mt-2">
                <fwb-button @click="savePlacement" color="green" :outline="isPlacementSaved">
                    <template v-if="isPlacementSaved"> Saved </template>
                    <template v-else> Save Placement </template>
                </fwb-button>
            </p>

        </template>
        <template #footer>
            <p class="text-base leading-relaxed text-gray-500 dark:text-gray-400">
                Currently, we use Mesh NoC 4x4 by default. More architecture configurations will be supported soon. If
                you
                want to change the architecture, you can click 'Edit Architecture' to manually change the layout using
                the
                draw.io service.
            </p>
        </template>
    </fwb-modal>
</template>
