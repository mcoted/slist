#ifndef SLIST_NATIVE_H
#define SLIST_NATIVE_H

#include "slist_types.H"

namespace slist
{
	struct context;
	
	parse_node_ptr ___define (context& ctx, const parse_node_ptr& root);
	parse_node_ptr ___lambda (context& ctx, const parse_node_ptr& root);
	parse_node_ptr ___cons   (context& ctx, const parse_node_ptr& root);
	parse_node_ptr ___list   (context& ctx, const parse_node_ptr& root);
	parse_node_ptr ___car    (context& ctx, const parse_node_ptr& root);
	parse_node_ptr ___cdr    (context& ctx, const parse_node_ptr& root);
	parse_node_ptr ___if     (context& ctx, const parse_node_ptr& root);
	parse_node_ptr ___length (context& ctx, const parse_node_ptr& root);
	parse_node_ptr ___empty  (context& ctx, const parse_node_ptr& root);
	parse_node_ptr ___add    (context& ctx, const parse_node_ptr& root);
}

#endif