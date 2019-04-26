/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Clifford Wolf <clifford@clifford.at>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "kernel/yosys.h"
#include "kernel/sigtools.h"
#include "passes/pmgen/split_shiftx_pm.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

void create_split_shiftx(split_shiftx_pm &pm)
{
	if (pm.st.shiftxB.empty())
		return;
	log_assert(pm.st.shiftx);
	SigSpec A = pm.st.shiftx->getPort("\\A");
	SigSpec Y = pm.st.shiftx->getPort("\\Y");
	const int A_WIDTH = pm.st.shiftx->getParam("\\A_WIDTH").as_int();
	const int Y_WIDTH = pm.st.shiftx->getParam("\\Y_WIDTH").as_int();
	log_assert(Y_WIDTH > 1);
	std::vector<SigBit> bits;
	bits.resize(A_WIDTH / Y_WIDTH);
	for (int i = 0; i < Y_WIDTH; ++i) {
		for (int j = 0; j < A_WIDTH/Y_WIDTH; ++j)
			bits[j] = A[j*Y_WIDTH + i];
		pm.module->addShiftx(NEW_ID, bits, pm.st.shiftxB, Y[i]);
	}
	pm.st.shiftx->unsetPort("\\Y");

	pm.autoremove(pm.st.shiftx);
	pm.autoremove(pm.st.macc);
}

struct BitblastShiftxPass : public Pass {
	BitblastShiftxPass() : Pass("split_shiftx", "Split up multi-bit $shiftx cells") { }
	void help() YS_OVERRIDE
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    split_shiftx [selection]\n");
		log("\n");
		log("Split up $shiftx cells where Y_WIDTH > 1, with consideration for any $macc\n");
		log("cells that may be driving their B inputs.\n");
		log("\n");
	}
	void execute(std::vector<std::string> args, RTLIL::Design *design) YS_OVERRIDE
	{
		log_header(design, "Executing SPLIT_SHIFTX pass.\n");

		size_t argidx;
		for (argidx = 1; argidx < args.size(); argidx++)
		{
			break;
		}
		extra_args(args, argidx, design);

		for (auto module : design->selected_modules())
			split_shiftx_pm(module, module->selected_cells()).run(create_split_shiftx);
	}
} BitblastShiftxPass;

PRIVATE_NAMESPACE_END