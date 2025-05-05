#!/bin/bash
 
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'
 
# iterate through all files in tests directory
for file in tests/*; do
    # get outputs from both parsers
    ref_output=$(./lexer "$file" | /afs/ir/class/cs143/bin/parser)
    my_output=$(./lexer "$file" | ./parser)
    
    # filter line numbers from outputs
    filtered_ref=$(echo "$ref_output" | grep -v '^\s*#[0-9]\+$')
    filtered_my=$(echo "$my_output" | grep -v '^\s*#[0-9]\+$')
    
    # compare filtered outputs
    if diff <(echo "$filtered_ref") <(echo "$filtered_my") >/dev/null; then
        # no difference - print filename and green check
        echo -e "${file} ${GREEN}✓${NC}"
    else
        # difference found - show outputs and diff
        echo -e "${RED}Difference found in ${file}:${NC}"
        echo -e "\n${RED}Reference parser output:${NC}"
        echo "$ref_output"
        echo -e "\n${RED}My parser output:${NC}"
        echo "$my_output"
        echo -e "\n${RED}Diff between outputs:${NC}"
        diff <(echo "$ref_output") <(echo "$my_output")
    fi
done
