import yaml

with open('./../data/parameters.yaml', 'r') as f:
    data = yaml.load(f, Loader=yaml.FullLoader)
    print(data)

main_cols = ["Run", "U_cool_ctrl_V_set", "U_cool_ctrl_V_out", "I_e_uA", "N_ion", "main_changes"]
main_titles = {
    "Run": "Run",
    "U_cool_ctrl_V_set": "$U_{\\mathrm{cool,ctrl,set}}$ [V]",
    "U_cool_ctrl_V_out": "$U_{\\mathrm{cool,ctrl,out}}$ [V]",
    "I_e_uA": "$I_e$ [uA]",
    "N_ion": "N$_{ion}$",
    "main_changes": "main changes",
}
secondary_cols = ["Run","U_eff_direct_V", "U_eff_sync_V", "f_RF_kHz", "h",
                  "t_precool_s", "r_cath", "U_c_V", "alpha"]
secondary_titles = {
    "Run": "Run",
    "U_eff_direct_V": "$U_{\\mathrm{eff},direct}$ [V]",
    "U_eff_sync_V": "$U_{\\mathrm{eff},sync}$ [V]",
    "f_RF_kHz": "$f_{RF}$ [kHz]",
    "h": "h",
    "t_precool_s": "$t_{precool}$ [s]",
    "r_cath": "$r_{cath}$",
    "U_c_V": "$U_c$ [V]",
    "alpha": "$\\alpha$"
}

def build_table_lines(data, columns):
    lines = []
    for ion_idx, (ion, runs) in enumerate(data.items()):
        print(ion_idx)
        print(ion)
        print(runs)
        if ion_idx > 0:
            lines.append(r"\hline")
        # Use list(runs.keys()) to preserve original YAML order
        run_ids = list(runs.keys())
        for idx, run_id in enumerate(run_ids):
            vals = runs[run_id]
            row = [ion if idx==0 else ""]
            # Force the run_id to be a str and zero-pad only when necessary
            run_str = str(run_id)
            # Just in case user gave mixed YAML, preserve 4-digit for all (if not already)
            if len(run_str) < 4:
                run_str = run_str.zfill(4)
            row.append(run_str)
            for c in columns[1:]:
                v = vals.get(c, None)
                row.append("" if v is None else v)
            lines.append(" & ".join(str(x) for x in row) + r" \\")
    return lines

def write_table(lines, titles, columns, fname, caption="Parameters"):
    n_col = len(columns) + 1  # +1 for Ion column
    header = r"""\begin{table}[h]
\centering
\begin{tabular}{l|""" + "c|"*(n_col-1) + r"""}
"""
    title_row = "Ion & " + " & ".join([titles[c] for c in columns]) + r" \\ \hline"
    footer = r"""\end{tabular}
\caption{""" + caption + r"""}
\end{table}
"""
    with open(fname, "w") as f:
        f.write(header)
        f.write(title_row + "\n")
        for l in lines:
            f.write(l + "\n")
        f.write(footer)
    print(f"Wrote {fname}")

main_lines = build_table_lines(data, main_cols)
sec_lines = build_table_lines(data, secondary_cols)

write_table(main_lines, main_titles, main_cols, "main_params_table.tex", caption="Main cooling force parameters")
write_table(sec_lines, secondary_titles, secondary_cols, "secondary_params_table.tex", caption="Secondary cooling force parameters")