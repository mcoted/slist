#ifndef SLIST_NATIVE_H
#define SLIST_NATIVE_H

#include "slist_types.H"

namespace slist
{
	struct context;
	
	node_ptr ___define  (context& ctx, const node_ptr& root);
	node_ptr ___lambda  (context& ctx, const node_ptr& root);
	node_ptr ___apply   (context& ctx, const node_ptr& root);
	node_ptr ___cons    (context& ctx, const node_ptr& root);
	node_ptr ___list    (context& ctx, const node_ptr& root);
	node_ptr ___car     (context& ctx, const node_ptr& root);
	node_ptr ___cdr     (context& ctx, const node_ptr& root);
	node_ptr ___begin   (context& ctx, const node_ptr& root);
	node_ptr ___if      (context& ctx, const node_ptr& root);
	node_ptr ___length  (context& ctx, const node_ptr& root);
	node_ptr ___empty   (context& ctx, const node_ptr& root);
	node_ptr ___print   (context& ctx, const node_ptr& root);
	node_ptr ___println (context& ctx, const node_ptr& root);
	node_ptr ___add     (context& ctx, const node_ptr& root);
	node_ptr ___sub     (context& ctx, const node_ptr& root);
}

#endif