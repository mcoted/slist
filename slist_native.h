#ifndef SLIST_NATIVE_H
#define SLIST_NATIVE_H

#include "slist_types.H"

namespace slist
{
	struct context;
	
	node_ptr native_eval     (context& ctx, const node_ptr& root);	
	node_ptr native_apply    (context& ctx, const node_ptr& root);
	node_ptr native_cons     (context& ctx, const node_ptr& root);
	node_ptr native_list     (context& ctx, const node_ptr& root);
	node_ptr native_car      (context& ctx, const node_ptr& root);
	node_ptr native_cdr      (context& ctx, const node_ptr& root);
	node_ptr native_quote    (context& ctx, const node_ptr& root);
	node_ptr native_lambda   (context& ctx, const node_ptr& root);
	node_ptr native_define   (context& ctx, const node_ptr& root);
	node_ptr native_set      (context& ctx, const node_ptr& root);
	node_ptr native_let      (context& ctx, const node_ptr& root);
	node_ptr native_begin    (context& ctx, const node_ptr& root);
	node_ptr native_if       (context& ctx, const node_ptr& root);
	node_ptr native_length   (context& ctx, const node_ptr& root);
	node_ptr native_empty    (context& ctx, const node_ptr& root);
	node_ptr native_print    (context& ctx, const node_ptr& root);
	node_ptr native_println  (context& ctx, const node_ptr& root);
	node_ptr native_eq       (context& ctx, const node_ptr& root);
	node_ptr native_equal    (context& ctx, const node_ptr& root);	
	node_ptr native_not      (context& ctx, const node_ptr& root);
	node_ptr native_is_pair  (context& ctx, const node_ptr& root);
	node_ptr native_is_bool  (context& ctx, const node_ptr& root);
	node_ptr native_is_int   (context& ctx, const node_ptr& root);
	node_ptr native_is_number(context& ctx, const node_ptr& root);
	node_ptr native_is_string(context& ctx, const node_ptr& root);
	node_ptr native_is_symbol(context& ctx, const node_ptr& root);
	node_ptr native_add      (context& ctx, const node_ptr& root);
	node_ptr native_sub      (context& ctx, const node_ptr& root);
	node_ptr native_mul      (context& ctx, const node_ptr& root);
	node_ptr native_div      (context& ctx, const node_ptr& root);
	node_ptr native_e        (context& ctx, const node_ptr& root);
	node_ptr native_ne       (context& ctx, const node_ptr& root);
	node_ptr native_lt       (context& ctx, const node_ptr& root);
	node_ptr native_gt       (context& ctx, const node_ptr& root);
	node_ptr native_le       (context& ctx, const node_ptr& root);
	node_ptr native_ge       (context& ctx, const node_ptr& root);	
	node_ptr native_assert   (context& ctx, const node_ptr& root);
	node_ptr native_run_test (context& ctx, const node_ptr& root);
}

#endif