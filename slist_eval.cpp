#include "slist_eval.h"
#include "slist_parser.h"
#include <iostream>

namespace
{
	slist::node_ptr eval_list(slist::context& ctx, const slist::node_ptr& root);
}

namespace slist
{
	node_ptr eval(context& ctx, const node_ptr& root)
	{
		switch (root->type)
		{
			case node_type::list:
				return eval_list(ctx, root);
			default: 
				break;
		}

		return nullptr;
	}
}

namespace
{
	slist::node_ptr eval_list(slist::context& ctx, const slist::node_ptr& root)
	{
		if (root->children.empty())
		{
			return root;
		}

		auto op_node = root->children[0];
		auto it = ctx.global_funcs.find(op_node->data);
		if (it != ctx.global_funcs.end())
		{
			return it->second(root);
		}

		return root;
	}
}