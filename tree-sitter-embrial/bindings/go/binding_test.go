package tree_sitter_embrial_test

import (
	"testing"

	tree_sitter "github.com/tree-sitter/go-tree-sitter"
	tree_sitter_embrial "github.com/tree-sitter/tree-sitter-embrial/bindings/go"
)

func TestCanLoadGrammar(t *testing.T) {
	language := tree_sitter.NewLanguage(tree_sitter_embrial.Language())
	if language == nil {
		t.Errorf("Error loading Embrial grammar")
	}
}
