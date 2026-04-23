#include "solver.hpp"
#include "rules.hpp"
#include <cstring>

// ── Gestão de Grid ────────────────────────────────────────────────────────────

Grid *clone_grid(const Grid *original) {
    if (!original) return nullptr;

    Grid *novo   = new Grid;
    novo->n      = original->n;
    novo->L      = original->L;
    int n        = original->n;

    novo->data   = new int[n * n];          // único malloc dos dados
    novo->cells  = new int*[n];             // array de ponteiros

    for (int i = 0; i < n; ++i)
        novo->cells[i] = novo->data + i * n;

    memcpy(novo->data, original->data, n * n * sizeof(int));

    return novo;
}

// copy_grid com loop: seguro para Grids externos que podem não ter data
static void copy_grid(Grid *dst, const Grid *src) {
    for (int i = 0; i < dst->n; ++i)
        for (int j = 0; j < dst->n; ++j)
            dst->cells[i][j] = src->cells[i][j];
}

// ── Solver sequencial ─────────────────────────────────────────────────────────

static int solve_seq(Grid *g, int start_row, int start_col, int *resolvido_global) {
    if (*resolvido_global) return 0;

    int row = -1, col = -1, encontrou_vazio = 0;

    for (int i = start_row; i < g->n && !encontrou_vazio; ++i) {
        int j_start = (i == start_row) ? start_col : 0;
        for (int j = j_start; j < g->n && !encontrou_vazio; ++j) {
            if (g->cells[i][j] == 0) { 
                row = i; 
                col = j; 
                encontrou_vazio = 1; 
            }
        }
    }

    if (!encontrou_vazio) return 1;

    for (int num = 1; num <= g->n; ++num) {
        if (is_valid(g, row, col, num)) {
            g->cells[row][col] = num;
            if (solve_seq(g, row, col, resolvido_global)) return 1;
            g->cells[row][col] = 0;
        }
    }
    return 0;
}

// ── Solver paralelo ───────────────────────────────────────────────────────────

static void solve_parallel(Grid *g, int depth, int start_row, int start_col, int *resolvido, Grid **solucao) {
    if (*resolvido) return;

    int row = -1, col = -1, encontrou_vazio = 0;

    for (int i = start_row; i < g->n && !encontrou_vazio; ++i) {
        int j_start = (i == start_row) ? start_col : 0;
        for (int j = j_start; j < g->n && !encontrou_vazio; ++j) {
            if (g->cells[i][j] == 0) { 
                row = i; 
                col = j; 
                encontrou_vazio = 1; 
            }
        }
    }

    if (!encontrou_vazio) {
        #pragma omp critical(solver_state)
        {
            if (!*resolvido) {
                *resolvido = 1;
                *solucao   = clone_grid(g);
            }
        }
        return;
    }

    if (depth < 4) {
            for (int num = 1; num <= g->n; ++num) {
                if (*resolvido) break;
                if (is_valid(g, row, col, num)) {
                    Grid *copia        = clone_grid(g);
                    copia->cells[row][col] = num;

                    #pragma omp task firstprivate(copia) shared(resolvido, solucao)
                    {
                        solve_parallel(copia, depth + 1, row, col, resolvido, solucao);
                        free_grid(copia);
                    }
                }
        }
    } else {
        if (solve_seq(g, row, col, resolvido)) {
            #pragma omp critical(solver_state)
            {
                if (!*resolvido) {
                    *resolvido = 1;
                    *solucao   = clone_grid(g);
                }
            }
        }
    }
}

// ── Ponto de entrada ──────────────────────────────────────────────────────────

int solve(Grid *g) {
    int   resolvido = 0;
    Grid *solucao   = nullptr;

    #pragma omp parallel
    #pragma omp single
    {
        solve_parallel(g, 0, 0, 0, &resolvido, &solucao);
    }

    if (resolvido && solucao) {
        copy_grid(g, solucao);
        free_grid(solucao);
        return 1;
    }
    return 0;
}