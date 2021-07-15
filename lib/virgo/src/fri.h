#ifndef __fri
#define __fri
#include "merkle_tree.h"
#include <utility>
#include "fieldElement.h"
#include "constants.h"
#include <vector>

namespace virgo {
    namespace fri {
        class commit_phase_data {
        public:
            __hhash_digest *merkle[max_fri_depth];
            int merkle_size[max_fri_depth];
            fieldElement *rs_codeword[max_fri_depth], *poly_coef[max_fri_depth];
            fieldElement *rs_codeword_msk[max_fri_depth];
            int *rs_codeword_mapping[max_fri_depth], *rs_codeword_msk_mapping[max_fri_depth];

            commit_phase_data() {
                for (int i = 0; i < max_fri_depth; ++i) {
                    merkle[i] = NULL;
                    rs_codeword[i] = NULL;
                    poly_coef[i] = NULL;
                    rs_codeword_msk[i] = NULL;
                    rs_codeword_msk_mapping[i] = NULL;
                    merkle_size[i] = 0;
                }
            }

            void delete_self() {
                for (int i = 0; i < max_fri_depth; ++i) {
                    if (merkle[i] != NULL)
                        delete[] merkle[i];
                    if (rs_codeword[i] != NULL)
                        delete[] rs_codeword[i];
                    if (poly_coef[i] != NULL)
                        delete[] poly_coef[i];
                    if (rs_codeword_msk[i] != NULL)
                        delete[] rs_codeword_msk[i];
                    if (rs_codeword_msk_mapping[i] != NULL)
                        delete[] rs_codeword_msk_mapping[i];
                    merkle[i] = NULL;
                    rs_codeword[i] = NULL;
                    poly_coef[i] = NULL;
                    rs_codeword_msk[i] = NULL;
                    rs_codeword_msk_mapping[i] = NULL;
                    merkle_size[i] = 0;
                }
            }

            ~commit_phase_data() {
                delete_self();
            }
        };

        extern int log_current_witness_size_per_slice, current_step_no, witness_bit_length_per_slice;
        extern commit_phase_data cpd;
        extern double __fri_timer;

        extern __hhash_digest *witness_merkle[2];
        //fieldElement *witness_rs_codeword[slice_number], *witness_poly_coef[slice_number];
        extern fieldElement *witness_rs_codeword_before_arrange[2][slice_number + 1];
        extern fieldElement *witness_rs_codeword_interleaved[2];

        extern int *witness_rs_mapping[2][slice_number + 1];
        extern fieldElement *L_group;
        extern bool *visited[max_bit_length];
        extern bool *visited_init[2];
        extern bool *visited_witness[2];
        extern fieldElement *virtual_oracle_witness;
        extern int *virtual_oracle_witness_mapping, *virtual_oracle_witness_msk_mapping;

        extern fieldElement *r_extended;
        extern __hhash_digest *leaf_hash[2];
        extern fieldElement *virtual_oracle_witness, *virtual_oracle_witness_msk;

        //Given private input, calculate the first oracle commitment
        __hhash_digest request_init_commit(const int bit_len, const int oracle_indicator);

        //request two values w^{pow0} and w^{pow1}, with merkle tree proof, where w is the root of unity and w^{pow0} and w^{pow1} are quad residue. Repeat ldt_repeat_num times, storing all result in vector
        std::pair<std::vector<std::pair<fieldElement, fieldElement> >, std::vector<__hhash_digest> >
        request_init_value_with_merkle(long long pow_0, long long pow_1, int &new_size, const int oracle_indicator);

        /*
         * request the merkle proof to lvl-th level oracle, at w^{pow}, will also return it's quad residue's proof.
         * returned value is unordered, meaning that one of them is the requested value and the other one is it's qual residue.
         */
        std::pair<std::vector<std::pair<fieldElement, fieldElement> >, std::vector<__hhash_digest> >
        request_step_commit(int lvl, long long pow, int &new_size);

        /*
         * Given fold parameter r, return the root of the merkle tree of next level.
         * TODO: Plan to remove slice_num, merge all merkle trees, leaf stores value.
         * Randomness are shared across all slices.
         */
        __hhash_digest commit_phase_step(fieldElement r);

        //return the final rs code since it's only constant size
        fieldElement *commit_phase_final();

        void delete_self();

        //Expand the public array given randomness
        void public_array_init(fieldElement *r, int len_r);
    }
}
#endif
