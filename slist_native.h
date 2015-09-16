#ifndef SLIST_NATIVE_H
#define SLIST_NATIVE_H

#include "slist_types.H"

namespace slist
{
	struct context;
	
	node_ptr ___define (context& ctx, const node_ptr& root);
	node_ptr ___list   (context& ctx, const node_ptr& root);
	node_ptr ___car    (context& ctx, const node_ptr& root);
	node_ptr ___cdr    (context& ctx, const node_ptr& root);
	node_ptr ___if     (context& ctx, const node_ptr& root);
	node_ptr ___length (context& ctx, const node_ptr& root);
	node_ptr ___empty  (context& ctx, const node_ptr& root);
	node_ptr ___add    (context& ctx, const node_ptr& root);
}

#endif