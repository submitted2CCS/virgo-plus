#include "poly_commit.h"

namespace virgo {
    fieldElement *poly_commit::all_pri_msk_arr;
    fieldElement *poly_commit::all_pub_msk_arr;
    fieldElement *poly_commit::inner_prod_evals;
    fieldElement *poly_commit::l_coef, *poly_commit::l_eval, *poly_commit::q_coef, *poly_commit::q_eval; //l for private input, q for public randomness
    fieldElement *poly_commit::lq_eval, *poly_commit::h_coef, *poly_commit::lq_coef, *poly_commit::h_eval;
    fieldElement *poly_commit::h_eval_arr;
    int poly_commit::l_coef_len, poly_commit::l_eval_len, poly_commit::q_coef_len, poly_commit::q_eval_len;
    int poly_commit::slice_size, poly_commit::slice_count, poly_commit::slice_real_ele_cnt;
    int poly_commit::mask_position_gap; //masks are positioned in specific way for efficiency
    bool poly_commit::pre_prepare_executed = false;
}
