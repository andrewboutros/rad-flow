<script setup lang="ts">
import { ref, watch, computed, onMounted } from 'vue';
import StackedChartWrapper from "./stackchartwrapper"
import { TemporalViewRendering } from "@/data"

interface Props {
  height: number,
  timeUnit: number,
  timeRange: [number, number], // TODO: determine the direction of control flow
  data: Array<TemporalViewRendering>,
  refresh: boolean, // edge triggering (both pos and neg)
};
const props = defineProps<Props>()
const emit = defineEmits(['timeRangeChanged', 'pktTypeChanged'/*TODO*/]);

// const timeRange = computed(() => props.timeRange);
const viewHeight = computed(() => props.height);
const xAxisTimeUnit = computed(() => props.timeUnit);
const chartData = computed(() => props.data);
const refreshTrigger = computed(() => props.refresh);

watch(refreshTrigger, () => render());
watch(viewHeight, () => render());

const chart = new StackedChartWrapper((from, to) => {
  emit('timeRangeChanged', [from, to]);
});

// watch(timeRange, ([from, to]) => {
//     // chart.moveBrush(from, to);
// })

watch(chartData, (data) => {
  chart.loadData(data, xAxisTimeUnit.value);
  render();
})

const divRef = ref(null);

function render() {
  const div = divRef.value! as HTMLDivElement;
  const width = div.parentElement?.clientWidth!;
  const height = viewHeight.value;
  div.replaceChildren(chart.getLegendDiv(), chart.getSVGElement(width, height));
}

onMounted(() => {
  chart.loadData(chartData.value, xAxisTimeUnit.value);
  render();
  // chart.moveBrush(0, 1);

  document.addEventListener('keyup', (ev) => {
    if (ev.key == "ArrowLeft") {
      chart.nextBrush(-1);
    } else if (ev.key == "ArrowRight") {
      chart.nextBrush(1);
    }
  });
})

</script>

<template>
  <div id="temporal-view" ref="divRef"></div>
</template>

<style scoped>
#temporal-view {
  user-select: none;
}
</style>
