#include "analyzer.h"
#include "grammar.h"

/**
 * @brief Finds a terminal identifier by terminal name.
 * @param g Parsed grammar.
 * @param name Terminal name to search.
 * @return Terminal id if found, otherwise -1.
 */
static int find_terminal_id(const grammar *g, const char *name)
{
	if (g == NULL || name == NULL) {
		return -1; // Invalid input
	}

	for (int i = 0; i < g->num_terminals; i++) {
        if (strcmp(g->terminals[i].symbol, name) == 0) {
            return i; // Terminal found, return its id
        }
    }

    return -1; // Terminal not found
}


/**
 * @brief Appends one symbol to a dynamically sized symbol array.
 * @param arr Target array pointer.
 * @param count Current element count; incremented on success.
 * @param text Symbol text to copy.
 * @param is_terminal Indicates terminal/non-terminal role.
 * @return true on success, false on allocation failure.
 */
static bool add_symbol_to_array(symbol **arr, int *count, const char *text, bool is_terminal)
{
	// Reallocating the array to hold one more symbol
	symbol *tmp = realloc(*arr, (*count + 1) * sizeof(symbol));
	if (tmp == NULL) {
		return false;  
	}

	*arr = tmp;

	// Duplicating the symbol text
	char *symbol_text = strdup(text);
	if (symbol_text == NULL) {
		*arr = realloc(*arr, (*count) * sizeof(symbol)); // revert to previous size on failure
		return false;  
	}

	// Filling the symbol metadata
	(*arr)[*count].symbol = symbol_text;
	(*arr)[*count].symbol_length = strlen(text);
	(*arr)[*count].is_terminal = is_terminal;

	(*count)++; // Incrementing the count of symbols

	return true;
}

/**
 * @brief Builds FIRST and nullable tables for all non-terminals.
 * @param g Parsed grammar.
 * @param first_table Output flattened table: non-terminal x terminal.
 * @param nullable Output nullable flags per non-terminal.
 * @param epsilon_id Output id of terminal "epsilon", or -1 if absent.
 * @return true when tables were built, false on invalid input or allocation error.
 */
static bool compute_first_tables(const grammar *g, bool **first_table, bool **nullable, int *epsilon_id)
{
	if (g == NULL || first_table == NULL || nullable == NULL || epsilon_id == NULL){
		return false; 
	}

	*first_table = NULL;
	*nullable = NULL;
	*epsilon_id = -1;
   
	// Allocating the FIRST table (non-terminals x terminals)
	*first_table = calloc(g->num_non_terminals * g->num_terminals, sizeof(bool));
	if (*first_table == NULL) {
		return false; 
	}

	// Allocating the nullable array (one bool per non-terminal)
	*nullable = calloc(g->num_non_terminals, sizeof(bool));
	if (*nullable == NULL) {
		free(*first_table);
		return false; 
	}

	*epsilon_id = find_terminal_id(g, "epsilon");

	bool changed = true;
	while (changed) {
		changed = false;

		for (int i = 0; i < g->num_productions; i++) {
			production *prod = &g->productions[i];
			int no_term_id = prod->non_terminal_id; 
		
            // [nullable] Check if the production is of the form X -> epsilon
            if (prod->production_length == 1 && prod->production_symbol_ids[0] == *epsilon_id) {
                if (!(*nullable)[no_term_id]) {
                    (*nullable)[no_term_id] = true;
                    changed = true;
                }
                continue;
            }

			/* [FIRST] CHeck each symbol in the production body until we find a non-nullable non-terminal
			   (X -> Y1 Y2 ... Yk) means FIRST(X) includes FIRST(Y1), and if Y1 is nullable, also includes FIRST(Y2), etc. */
			
			bool all_nullable = true;

			for (int j = 0; j < prod->production_length; j++) {
				int sym_id = prod->production_symbol_ids[j];
				if (sym_id < g->num_terminals) {
					// [FIRST] Symbol is a terminal, add it to FIRST(X)
					int term_id = sym_id;
					if (!(*first_table)[no_term_id * g->num_terminals + term_id]) {
						(*first_table)[no_term_id * g->num_terminals + term_id] = true;
						changed = true;
					}
					all_nullable = false;
					break; 
				} else {
					// [FIRST] Symbol is a non-terminal, add its FIRST set to FIRST(X)
					int next_no_term_id = sym_id - g->num_terminals;
					for (int t = 0; t < g->num_terminals; t++) {
						if ((*first_table)[next_no_term_id * g->num_terminals + t]) {
							if (!(*first_table)[no_term_id * g->num_terminals + t]) {
								(*first_table)[no_term_id * g->num_terminals + t] = true;
								changed = true;
							}
						}
					
					}
					if (!(*nullable)[next_no_term_id]) {
						all_nullable = false;	
						break; // Stop if the non-terminal is not nullable
					}
				}
			}
			
			// [nullable] If all symbols were nullable, X is also nullable
			if (all_nullable && !(*nullable)[no_term_id]) {
				(*nullable)[no_term_id] = true;
				changed = true;
			}
		}

	}

	return true;
}



/**
 * @brief Builds FOLLOW table for all non-terminals.
 * @param g Parsed grammar.
 * @param first_table FIRST table from compute_first_tables.
 * @param nullable Nullable flags from compute_first_tables.
 * @param epsilon_id Terminal id for "epsilon", or -1.
 * @param out_follow Output flattened table: non-terminal x (terminals + '$').
 * @param out_follow_cols Output number of columns for out_follow.
 * @return true on success, false on allocation error or invalid input.
 */
static bool compute_follow_table(
    const grammar *g,
    const bool *first_table,
    const bool *nullable,
    int epsilon_id,
    bool **out_follow,
    int *out_follow_cols)
{
    if (g == NULL || first_table == NULL || nullable == NULL || 
        out_follow == NULL || out_follow_cols == NULL) {
        return false;
    }
    
    int num_nt = g->num_non_terminals;
    int num_t = g->num_terminals;
    int num_cols = num_t + 1;  // Terminals '$'
    
    *out_follow = calloc(num_nt * num_cols, sizeof(bool));
    if (*out_follow == NULL) {
        return false;
    }
    *out_follow_cols = num_cols;
    
    // initial symbol has '$'
    int dollar_col = num_t;
    (*out_follow)[0 * num_cols + dollar_col] = true;
    
    bool changed = true;
    while (changed) {
        changed = false;
        
        // for all productions
        for (int i = 0; i < g->num_productions; i++) {
            production *prod = &g->productions[i];
            int nt_left = prod->non_terminal_id;
            
            // right side looking for non terminals
            for (int j = 0; j < prod->production_length; j++) {
                int sym_id = prod->production_symbol_ids[j];
                
				// Non terminals
                if (sym_id >= g->num_terminals) {
                    int nt_right = sym_id - g->num_terminals;  // X in "A -> a X B"
                    
                    // j+1 position after B
                    int k = j + 1;
                    
                    // Adds first(B) to Follow(X)
                    // Process all symbols of B until find one non nullable
                    bool all_beta_nullable = true;
                    
                    while (k < prod->production_length) {
                        int next_sym = prod->production_symbol_ids[k];
                        
                        if (next_sym < g->num_terminals) {
                            // B starts with a terminal, adding it and finishing
                            int term_id = next_sym;
                            int idx = nt_right * num_cols + term_id;
                            if (!(*out_follow)[idx]) {
                                (*out_follow)[idx] = true;
                                changed = true;
                            }
                            all_beta_nullable = false;
                            break;
                        } else {
                            // B starts with a non terminal
                            int beta_nt = next_sym - g->num_terminals;
                            
                            // Adds FIRST(beta_nt) - {epsilon } to FOLLOW(X)
                            for (int t = 0; t < num_t; t++) {
                                if (t == epsilon_id) continue;
                                
                                if (first_table[beta_nt * num_t + t]) {
                                    int idx = nt_right * num_cols + t;
                                    if (!(*out_follow)[idx]) {
                                        (*out_follow)[idx] = true;
                                        changed = true;
                                    }
                                }
                            }
                            
                            // If beta_nt isn't nullable, stop
                            if (!nullable[beta_nt]) {
                                all_beta_nullable = false;
                                break;
                            }
                            // if nullable continue with next symbol
                            k++;
                        }
                    }
                    
                    // If B is nullable , FOLLOW(A) in FOLLOW(X)
                    if (all_beta_nullable || k == prod->production_length) {
                        for (int col = 0; col < num_cols; col++) {
                            if ((*out_follow)[nt_left * num_cols + col]) {
                                int idx = nt_right * num_cols + col;
                                if (!(*out_follow)[idx]) {
                                    (*out_follow)[idx] = true;
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return true;
}

/**
 * @brief Collects FIRST symbols for one non-terminal from the computed table.
 * @param g Parsed grammar.
 * @param non_terminal_id Non-terminal index.
 * @param first_table FIRST table.
 * @param nullable Nullable flags.
 * @param epsilon_id Terminal id for "epsilon", or -1.
 * @param out_first Output array with FIRST symbols.
 * @return Number of collected symbols, or 0 on error.
 */
static int collect_first_for_non_terminal(
	const grammar *g,
	int non_terminal_id,
	const bool *first_table,
	const bool *nullable,
	int epsilon_id,
	symbol **out_first)
{
	if (g == NULL || first_table == NULL || nullable == NULL || out_first == NULL) {
		return 0;
	}

	if (non_terminal_id < 0 || non_terminal_id >= g->num_non_terminals) {
		return 0; 
	}

	*out_first = NULL;
	int symbol_count = 0;

	// Read the FIRST row for the non-terminal
	for (int t = 0; t < g->num_terminals; t++) {
		if (first_table[non_terminal_id * g->num_terminals + t]) {
			if (!add_symbol_to_array(out_first, &symbol_count, g->terminals[t].symbol, true)) {
				free_symbol_array(*out_first, symbol_count);
				*out_first = NULL;
				return 0; 
			}
		}
	}

	// If nullable, also include epsilon
    if (nullable[non_terminal_id] && epsilon_id >= 0 && epsilon_id < g->num_terminals) {
        if (!add_symbol_to_array(out_first, &symbol_count, g->terminals[epsilon_id].symbol, true)) {
            free_symbol_array(*out_first, symbol_count);
            *out_first = NULL;
            return 0;
        }
    }

	return symbol_count;

}

/**
 * @brief Collects FOLLOW symbols for one non-terminal from the computed table.
 * @param g Parsed grammar.
 * @param non_terminal_id Non-terminal index.
 * @param follow_table FOLLOW table.
 * @param follow_cols Number of columns in follow_table.
 * @param out_follow Output array with FOLLOW symbols.
 * @return Number of collected symbols, or 0 on error.
 */
static int collect_follow_for_non_terminal(
	const grammar *g,
	int non_terminal_id,
	const bool *follow_table,
	int follow_cols,
	symbol **out_follow)
{
	// TODO: Read one FOLLOW row, append terminal symbols, and include end marker '$' when present.
}

/**
 * @brief Computes FIRST set for one non-terminal by index.
 * @param g Parsed grammar.
 * @param non_terminal_id Non-terminal index in g->non_terminals.
 * @param out_first Output array with FIRST symbols.
 * @return Number of symbols in out_first, or 0 on error.
 */
int compute_first_for_non_terminal(const grammar *g, int non_terminal_id, symbol **out_first)
{
	// TODO: Validate inputs, compute shared FIRST tables, and collect FIRST for the requested non-terminal.
	if (g == NULL || non_terminal_id < 0 || non_terminal_id >= g->num_non_terminals || out_first == NULL) {
		return 0;
	}
	bool *first_table = NULL;
    bool *nullable = NULL;
	int epsilon_id;

	if (!compute_first_tables(g, &first_table, &nullable, &epsilon_id)) {
        return 0;
    }

	int count = collect_first_for_non_terminal(g, non_terminal_id, first_table, nullable, epsilon_id, out_first);
	
	free(first_table);
    free(nullable);

	return count;
	
}

/**
 * @brief Computes FOLLOW set for one non-terminal by index.
 * @param g Parsed grammar.
 * @param non_terminal_id Non-terminal index in g->non_terminals.
 * @param out_follow Output array with FOLLOW symbols.
 * @return Number of symbols in out_follow, or 0 on error.
 */
int compute_follow_for_non_terminal(const grammar *g, int non_terminal_id, symbol **out_follow)
{
    if (g == NULL || non_terminal_id < 0 || non_terminal_id >= g->num_non_terminals || out_follow == NULL) {
        return 0;
    }

    bool *first_table = NULL;
    bool *nullable = NULL;
    bool *follow_table = NULL;
    int epsilon_id;
    int follow_cols;
    
    if (!compute_first_tables(g, &first_table, &nullable, &epsilon_id)) {
        return 0;
    }

    if (!compute_follow_table(g, first_table, nullable, epsilon_id, &follow_table, &follow_cols)) {
        free(first_table);
        free(nullable);
        return 0;
    }
    
    int count = collect_follow_for_non_terminal(g, non_terminal_id, follow_table, follow_cols, out_follow);

    free(first_table);
    free(nullable);
    free(follow_table);

    return count;
}

/**
 * @brief Computes FIRST set for the start symbol.
 * @param g Parsed grammar.
 * @param out_first Output array with FIRST(start) terminals.
 * @return Number of symbols in out_first, or 0 on error.
 */
int compute_first_for_start_symbol(const grammar *g, symbol **out_first)
{
	if(g == NULL || g->num_non_terminals <= 0) {
		return 0;
	}
	compute_first_for_non_terminal(g,0,out_first);
}

/**
 * @brief Computes FOLLOW set for the start symbol.
 * @param g Parsed grammar.
 * @param out_follow Output array with FOLLOW(start) terminals.
 * @return Number of symbols in out_follow, or 0 on error.
 */
int compute_follow_for_start_symbol(const grammar *g, symbol **out_follow)
{
	if(g == NULL || g->num_non_terminals <= 0) {
		return 0;
	}
	compute_follow_for_non_terminal(g, 0,out_follow);
}

/**
 * @brief Frees a symbol array and each duplicated symbol string.
 * @param symbols Symbol array to release.
 * @param count Number of initialized entries.
 * @return This function does not return a value.
 */
void free_symbol_array(symbol *symbols, int count)
{
    if (symbols == NULL || count <= 0) {
        return;
    }
    
    for (int i = 0; i < count; i++) {
        if (symbols[i].symbol != NULL) {
            free(symbols[i].symbol);
            symbols[i].symbol = NULL;
        }
    }
    
    free(symbols);
}
