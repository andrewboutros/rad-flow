import math
import random
import os
import glob
import numpy as np

# Input parameters
model_csv = "ab_small.csv"
read_bytewidth = 64
element_bytewidth = 2
hbm_channels = 8
hbm_channel_words = 1 * 1024 * 1024 * 1024 / read_bytewidth
ddr_channels = 0
ddr_channel_words = 8 * 1024 * 1024 * 1024 / read_bytewidth
num_test_inputs = 1

# MLP parameters
native_dim = int(read_bytewidth / element_bytewidth)
num_layers = 3
hidden_dims = [1024, 512, 256]
num_mvms = [1, 1, 1]

# Model parsing
table_info = []
smallest_table_bytewidth = 8
input_dim = 0

# Memory allocation
hbm_channels_used_words = np.zeros(hbm_channels, dtype=int)
hbm_channels_rounds = np.zeros(hbm_channels, dtype=int)
ddr_channels_used_words = np.zeros(ddr_channels, dtype=int)
ddr_channels_rounds = np.zeros(ddr_channels, dtype=int)
tables_per_ddr_channel = {}
tables_per_hbm_channel = {}
base_addr_per_ddr_channel = {}
base_addr_per_hbm_channel = {}

# Testing
test_input_data = []
test_input_base_addr = []
test_input_target_ch = []
test_feature_interaction_outputs = []
test_golden_outputs = []
mem_contents_per_channel = [{} for channel in range(ddr_channels + hbm_channels)]

tobin = lambda x, count=8: "".join(
    map(lambda y: str((x >> y) & 1), range(count - 1, -1, -1))
)


def get_table_id(table):
    return table[0]


def get_table_vector_length(table):
    return table[1]


def get_table_vector_length_by_id(all_tables, id):
    for table in all_tables:
        if get_table_id(table) == id:
            return get_table_vector_length(table)
    return -1


def get_table_entries(table):
    return table[2]


def get_table_entries_by_id(all_tables, id):
    for table in all_tables:
        if get_table_id(table) == id:
            return get_table_entries(table)
    return -1


def get_table_words(table):
    return table[3]


def get_table_channel_type(table):
    if len(table) < 5:
        return -1
    return table[4]


def get_table_channel_id(table):
    if len(table) < 5:
        return -1
    return table[5]


def parse_dlrm_description(filename):
    global smallest_table_bytewidth
    global input_dim
    f = open(filename, "r")
    lines = f.readlines()
    id = 0
    for line in lines:
        if line[0] == "#":
            continue
        line_split = line.split(",")
        line_split = [eval(i) for i in line_split]
        input_dim += line_split[0]
        if line_split[0] * element_bytewidth < smallest_table_bytewidth:
            smallest_table_bytewidth = line_split[0] * element_bytewidth
        words_per_entry = int(math.ceil(1.0 * line_split[0] / read_bytewidth))
        line_split.append(words_per_entry * line_split[1])
        line_split.insert(0, id)
        id = id + 1
        table_info.append(line_split)
    f.close()


def sort_tables():
    table_info.sort(key=lambda x: x[3], reverse=True)


def greedy_allocation():
    round_id = 1
    for table in table_info:
        allocated = False
        while not (allocated):
            for ch in range(ddr_channels):
                rem_words = ddr_channel_words - ddr_channels_used_words[ch]
                if (ddr_channels_rounds[ch] < round_id) and (
                    rem_words >= get_table_words(table)
                ):
                    if ch in tables_per_ddr_channel:
                        tables_per_ddr_channel[ch].append(get_table_id(table))
                        base_addr_per_ddr_channel[ch].append(
                            ddr_channels_used_words[ch]
                        )
                    else:
                        tables_per_ddr_channel[ch] = [get_table_id(table)]
                        base_addr_per_ddr_channel[ch] = [ddr_channels_used_words[ch]]
                    ddr_channels_used_words[ch] += get_table_words(table)
                    ddr_channels_rounds[ch] += 1
                    allocated = True
                    table.append(1)
                    table.append(ch)

                    break

            if not (allocated):
                for ch in range(hbm_channels):
                    rem_words = hbm_channel_words - hbm_channels_used_words[ch]
                    if (hbm_channels_rounds[ch] < round_id) and (
                        rem_words >= get_table_words(table)
                    ):
                        if ch in tables_per_hbm_channel:
                            tables_per_hbm_channel[ch].append(get_table_id(table))
                            base_addr_per_hbm_channel[ch].append(
                                hbm_channels_used_words[ch]
                            )
                        else:
                            tables_per_hbm_channel[ch] = [get_table_id(table)]
                            base_addr_per_hbm_channel[ch] = [
                                hbm_channels_used_words[ch]
                            ]
                        hbm_channels_used_words[ch] += get_table_words(table)
                        hbm_channels_rounds[ch] += 1
                        allocated = True
                        table.append(0)
                        table.append(ch)
                        break

            if not (allocated):
                round_id += 1


def print_dlrm_description():
    print("Embedding Tables (sorted):")
    print("+----+----+-----------+")
    print("| #  | V  |  Entries  |")
    print("|----|----|-----------|")
    for row in table_info:
        print("| {:>2} | {:>2} | {:>9} |".format(*row))
    print("+----+----+-----------+")


def print_allocation():
    ddr_total_mb = 0
    hbm_total_mb = 0
    print("\nDDR Channel Allocations:")
    ddr_table = [ddr_channels_rounds, ddr_channels_used_words]
    ddr_table = np.transpose(ddr_table).tolist()
    print("+----+-----------+--------+----------+")
    print("| R  | Mem Words |   %    | Size(MB) |")
    print("|----|-----------|--------|----------|")
    for row in ddr_table:
        row.append(1.0 * row[1] / ddr_channel_words * 100)
        size_mb = 1.0 * row[1] * read_bytewidth / 1024 / 1024
        ddr_total_mb += size_mb
        row.append(size_mb)
        print("| {:>2} | {:>9} | {:5.2f}% | {:8.2f} |".format(*row))
    print("+----+-----------+--------+----------+")
    print("\nHBM Channel Allocations:")
    hbm_table = [hbm_channels_rounds, hbm_channels_used_words]
    hbm_table = np.transpose(hbm_table).tolist()
    print("+----+-----------+--------+----------+")
    print("| R  | Mem Words |   %    | Size(MB) |")
    print("|----|-----------|--------|----------|")
    for row in hbm_table:
        row.append(1.0 * row[1] / hbm_channel_words * 100)
        size_mb = 1.0 * row[1] * read_bytewidth / 1024 / 1024
        hbm_total_mb += size_mb
        row.append(size_mb)
        print("| {:>2} | {:>9} | {:5.2f}% | {:8.2f} |".format(*row))
    print("+----+-----------+--------+----------+")
    print("Total DDR memory footprint = {:.2f} MB".format(ddr_total_mb))
    print("Total HBM memory footprint = {:.2f} MB".format(hbm_total_mb))
    print("Total memory footprint = {:.2f} MB".format(ddr_total_mb + hbm_total_mb))
    print("\n")
    print("DDR Tables per channel:")
    for ch in tables_per_ddr_channel:
        print("{:>2} : ".format(ch), end="")
        for i in range(len(tables_per_ddr_channel[ch])):
            print(
                "{:>3} ({:>9}) ({:>2})".format(
                    tables_per_ddr_channel[ch][i],
                    base_addr_per_ddr_channel[ch][i],
                    int(
                        get_table_vector_length_by_id(
                            table_info, tables_per_ddr_channel[ch][i]
                        )
                        * element_bytewidth
                        / smallest_table_bytewidth
                    ),
                ),
                end="",
            )
        print("")
    print("\n")
    print("HBM Tables per channel:")
    for ch in tables_per_hbm_channel:
        print("{:>2} : ".format(ch), end="")
        for i in range(len(tables_per_hbm_channel[ch])):
            print(
                "{:>3} ({:>9}) ({:>2})".format(
                    tables_per_hbm_channel[ch][i],
                    base_addr_per_hbm_channel[ch][i],
                    int(
                        get_table_vector_length_by_id(
                            table_info, tables_per_hbm_channel[ch][i]
                        )
                        * element_bytewidth
                        / smallest_table_bytewidth
                    ),
                ),
                end="",
            )
        print("")
    print("\n")


def generate_embedding_lookup_inputs(num_inputs):
    f = open("embedding_indecies.in", "w")
    f.write(str(len(table_info)) + "\n")
    for i in range(num_inputs):
        input_vec = []
        target_ch = []
        base_addr = []
        round_id = 0
        done = False
        table_count = 0
        while not (done):
            for ch in tables_per_ddr_channel:
                if round_id < len(tables_per_ddr_channel[ch]):
                    table_id = tables_per_ddr_channel[ch][round_id]
                    limit = get_table_entries_by_id(table_info, table_id) - 1
                    input_vec.append(random.randint(0, limit) * read_bytewidth)
                    target_ch.append(ch)
                    base_addr.append(
                        base_addr_per_ddr_channel[ch][round_id] * read_bytewidth
                    )
                    vector_length = get_table_vector_length_by_id(table_info, table_id)
                    mem_addr = base_addr[-1] + input_vec[-1]
                    mem_contents_per_channel[ch][mem_addr] = [
                        random.randint(-1, 1) for i in range(vector_length)
                    ]
                    table_count += 1
            for ch in tables_per_hbm_channel:
                if round_id < len(tables_per_hbm_channel[ch]):
                    table_id = tables_per_hbm_channel[ch][round_id]
                    limit = get_table_entries_by_id(table_info, table_id) - 1
                    input_vec.append(random.randint(0, limit) * read_bytewidth)
                    target_ch.append(ddr_channels + ch)
                    base_addr.append(
                        base_addr_per_hbm_channel[ch][round_id] * read_bytewidth
                    )
                    vector_length = get_table_vector_length_by_id(table_info, table_id)
                    mem_addr = base_addr[-1] + input_vec[-1]
                    mem_contents_per_channel[ddr_channels + ch][mem_addr] = [
                        random.randint(-1, 1) for i in range(vector_length)
                    ]
                    table_count += 1
            round_id += 1
            done = table_count == len(table_info)
        test_input_data.append(input_vec)
        test_input_base_addr.append(base_addr)
        test_input_target_ch.append(target_ch)
        for j in input_vec:
            f.write(str(j) + " ")
        f.write("\n")
        for j in target_ch:
            f.write(str(j) + " ")
        f.write("\n")
        for j in base_addr:
            f.write(str(j) + " ")
        f.write("\n")
    f.close()


def generate_mem_channel_contents():
    # Prepare instruction MIFs directory
    if not (os.path.exists("./embedding_tables")):
        os.mkdir("embedding_tables")
    else:
        files = glob.glob("embedding_tables/*.dat")
        for file in files:
            os.remove(file)

    for c in range(ddr_channels + hbm_channels):
        f = open("embedding_tables/channel_" + str(c) + ".dat", "w")
        for addr in mem_contents_per_channel[c]:
            content = mem_contents_per_channel[c][addr]
            string_content = ""
            byte_count = 0
            for e in content:
                string_content = tobin(e, element_bytewidth * 8) + string_content
                byte_count += element_bytewidth
            for i in range(read_bytewidth - byte_count):
                string_content = tobin(0, element_bytewidth * 8) + string_content
            f.write(str(addr) + " " + string_content + "\n")
        f.close()


def pop_one_hot(fifo_ids):
    one_hot = ""
    for i in range(ddr_channels + hbm_channels):
        one_hot += "0"
    for id in fifo_ids:
        one_hot = one_hot[:id] + "1" + one_hot[id + 1 :]
    return one_hot


def generate_feature_interaction_instructions():
    if not (os.path.exists("./instructions")):
        os.mkdir("instructions")
    else:
        files = glob.glob("instructions/*.inst")
        for file in files:
            os.remove(file)

    global smallest_table_bytewidth
    f = open("instructions/feature_interaction.inst", "w")
    round_id = 0
    table_count = 0
    total_flush_count = 0
    flush_counters = np.zeros(ddr_channels + hbm_channels, dtype=int)
    total_bytes = 0
    while table_count < len(table_info):
        for c in tables_per_ddr_channel:
            if round_id < len(tables_per_ddr_channel[c]):
                vector_length = get_table_vector_length_by_id(
                    table_info, tables_per_ddr_channel[c][round_id]
                )
                num_pops = int(
                    vector_length * element_bytewidth / smallest_table_bytewidth
                )
                total_bytes += vector_length * element_bytewidth
                for p in range(num_pops):
                    fifo_ids = [c]
                    for fc in range(len(flush_counters)):
                        if flush_counters[fc] != 0:
                            fifo_ids.append(fc)
                            flush_counters[fc] -= 1
                            total_flush_count -= 1
                    f.write(str(c + 1) + " " + pop_one_hot(fifo_ids) + "\n")
                num_flushes = (read_bytewidth / smallest_table_bytewidth) - num_pops
                total_flush_count += num_flushes
                flush_counters[c] += num_flushes
                table_count += 1

        for ch in tables_per_hbm_channel:
            c = ch + ddr_channels
            if round_id < len(tables_per_hbm_channel[ch]):
                vector_length = get_table_vector_length_by_id(
                    table_info, tables_per_hbm_channel[ch][round_id]
                )
                num_pops = int(
                    vector_length * element_bytewidth / smallest_table_bytewidth
                )
                total_bytes += vector_length * element_bytewidth
                for p in range(num_pops):
                    fifo_ids = [c]
                    for fc in range(len(flush_counters)):
                        if flush_counters[fc] != 0:
                            fifo_ids.append(fc)
                            flush_counters[fc] -= 1
                            total_flush_count -= 1
                    f.write(str(c + 1) + " " + pop_one_hot(fifo_ids) + "\n")
                num_flushes = (read_bytewidth / smallest_table_bytewidth) - num_pops
                total_flush_count += num_flushes
                flush_counters[c] += num_flushes
                table_count += 1

        round_id += 1

    if total_bytes % read_bytewidth > 0:
        remaining_bytes = read_bytewidth - (total_bytes % read_bytewidth)
    else:
        remaining_bytes = 0
    padding_words = int(remaining_bytes / smallest_table_bytewidth)
    while total_flush_count > 0:
        fifo_ids = []
        for fc in range(len(flush_counters)):
            if flush_counters[fc] != 0:
                fifo_ids.append(fc)
                flush_counters[fc] -= 1
                total_flush_count -= 1
        if padding_words > 0:
            f.write(
                str(ddr_channels + hbm_channels + 1)
                + " "
                + pop_one_hot(fifo_ids)
                + "\n"
            )
            padding_words -= 1
        else:
            f.write("0 " + pop_one_hot(fifo_ids) + "\n")

    while padding_words > 0:
        f.write(str(ddr_channels + hbm_channels + 1) + " " + pop_one_hot([]) + "\n")
        padding_words -= 1

    f.close()


def generate_feature_interaction_outputs():
    f = open("feature_interaction.out", "w")
    feature_interaction_vector_length = 0
    for table in table_info:
        feature_interaction_vector_length += get_table_vector_length(table)
    total_num_outputs = len(test_input_data) * int(
        feature_interaction_vector_length * element_bytewidth / read_bytewidth
    )
    f.write(str(total_num_outputs) + "\n")
    for input_id in range(len(test_input_data)):
        output_vec = []
        for idx_id in range(len(test_input_data[input_id])):
            idx = test_input_data[input_id][idx_id]
            base = test_input_base_addr[input_id][idx_id]
            ch = test_input_target_ch[input_id][idx_id]
            if ch >= ddr_channels:
                ch += ddr_channels
            mem_content = mem_contents_per_channel[ch][base + idx]
            for e in mem_content:
                output_vec.append(e)
        reshaped_output_vec = np.reshape(
            output_vec,
            (
                int(len(output_vec) / int((read_bytewidth / element_bytewidth))),
                int(read_bytewidth / element_bytewidth),
            ),
        )
        for o in reshaped_output_vec:
            for e in o:
                f.write(str(e) + " ")
            f.write("\n")
        test_feature_interaction_outputs.append(output_vec)
    f.close()


def generate_mlp_weights():
    # Generate random padded weight matrices
    padded_weights = []
    for l in range(num_layers):
        num_mvms_in = num_mvms[l]
        if l == num_layers - 1:
            num_mvms_out = num_mvms[0]
        else:
            num_mvms_out = num_mvms[l + 1]
        if l == 0:
            layer_input_dim = input_dim
        else:
            layer_input_dim = hidden_dims[l - 1]
        padded_dimx = int(
            math.ceil(layer_input_dim * 1.0 / native_dim / num_mvms_in)
            * native_dim
            * num_mvms_in
        )
        padded_dimy = int(
            math.ceil(hidden_dims[l] * 1.0 / native_dim / num_mvms_out)
            * native_dim
            * num_mvms_out
        )
        padded_weights.append(np.zeros(shape=(padded_dimy, padded_dimx), dtype=int))
        padded_weights[l][: hidden_dims[l], :layer_input_dim] = np.random.randint(
            -2, 2, size=(hidden_dims[l], layer_input_dim)
        )

    # Prepare weight MIFs directory
    if not (os.path.exists("./mvm_weights")):
        os.mkdir("mvm_weights")
    else:
        files = glob.glob("mvm_weights/*.dat")
        for file in files:
            os.remove(file)

    # Write weight MIFs
    for l in range(num_layers):
        layer_mvms = num_mvms[l]
        mvm_idx = 0
        limx = int(padded_weights[l].shape[1] / native_dim)
        limy = int(padded_weights[l].shape[0] / native_dim)
        mifs = []
        for m in range(layer_mvms):
            mifs.append([])
            for d in range(native_dim):
                mifs[m].append(
                    open(
                        "mvm_weights/layer"
                        + str(l)
                        + "_mvm"
                        + str(m)
                        + "_dot"
                        + str(d)
                        + ".dat",
                        "w",
                    )
                )

        for i in range(limx):
            for j in range(limy):
                for d in range(native_dim):
                    for e in range(native_dim):
                        mifs[mvm_idx][d].write(
                            str(
                                padded_weights[l][(j * native_dim) + d][
                                    (i * native_dim) + e
                                ]
                            )
                            + " "
                        )
                    mifs[mvm_idx][d].write("\n")
            if mvm_idx == layer_mvms - 1:
                mvm_idx = 0
            else:
                mvm_idx = mvm_idx + 1

        for mvm_mifs in mifs:
            for mif in mvm_mifs:
                mif.close()
    return padded_weights


def generate_mvm_instructions(padded_weights):
    # Generate instruction MIFs
    # en, jump, reduce, accum, accum_en, release, raddr, last, dest_layer, dest_mvm
    for l in range(num_layers):
        layer_mvms = num_mvms[l]
        limx = int(padded_weights[l].shape[1] / native_dim / layer_mvms)
        limy = int(padded_weights[l].shape[0] / native_dim)
        for m in range(layer_mvms):
            inst_mif = open(
                "instructions/layer" + str(l) + "_mvm" + str(m) + ".inst", "w"
            )
            for i in range(limx):
                for j in range(limy):
                    if (l == num_layers - 1) and (m == layer_mvms - 1):
                        dest_layer = 0
                        dest_mvm = 0
                    elif m == layer_mvms - 1:
                        dest_layer = l + 2
                        dest_mvm = j % num_mvms[l + 1]
                    else:
                        dest_layer = l + 1
                        dest_mvm = m + 1
                    inst_mif.write("1 0 ")  # en, jump
                    if m == 0 or i < limx - 1:
                        inst_mif.write("0 ")  # reduce
                    else:
                        inst_mif.write("1 ")  # reduce
                    if i == 0:
                        inst_mif.write(str(j) + " 0 ")  # accum, accum_en
                    else:
                        inst_mif.write(str(j) + " 1 ")  # accum, accum_en
                    if i == limx - 1:
                        inst_mif.write("1 ")  # release
                    else:
                        inst_mif.write("0 ")  # release
                    inst_mif.write(str(i * limy + j) + " ")  # raddr
                    if j == limy - 1:
                        inst_mif.write("1 ")  # last
                    else:
                        inst_mif.write("0 ")  # last
                    inst_mif.write(
                        str(dest_layer) + " " + str(dest_mvm) + "\n"
                    )  # dest_layer, dest_mvm
            inst_mif.write("1 1 0 0 0 0 0 0 0 0\n")
            inst_mif.close()


def generate_mlp_outputs(padded_weights):
    # Compute test outputs
    test_inputs = np.transpose(test_feature_interaction_outputs)
    test_outputs = np.dot(padded_weights[0], test_inputs)
    # test_outputs = np.maximum(test_outputs, np.zeros(shape=test_outputs.shape, dtype=int))
    for l in range(1, num_layers):
        test_outputs = np.dot(padded_weights[l], test_outputs)
        # test_outputs = np.maximum(test_outputs, np.zeros(shape=test_outputs.shape, dtype=int))
    test_outputs = np.transpose(test_outputs)

    # Generate test output MIFs
    output_file = open("./mlp.out", "w")
    for o in range(test_outputs.shape[0]):
        for c in range(int(test_outputs.shape[1] / native_dim)):
            for e in range(native_dim):
                output_file.write(str(test_outputs[o][(c * native_dim) + e]) + " ")
            output_file.write("\n")
    output_file.close()


def generate_mvms_config():
    # Generate layer/MVM configuration
    config_file = open("./mvms.config", "w")
    config_file.write(str(num_layers) + " ")
    for mvm_count in num_mvms:
        config_file.write(str(mvm_count) + " ")
    config_file.close()


parse_dlrm_description(model_csv)
sort_tables()
print_dlrm_description()
greedy_allocation()
print_allocation()
generate_embedding_lookup_inputs(num_test_inputs)
generate_mem_channel_contents()
generate_feature_interaction_instructions()
generate_feature_interaction_outputs()
padded_weights = generate_mlp_weights()
generate_mvm_instructions(padded_weights)
generate_mlp_outputs(padded_weights)
generate_mvms_config()