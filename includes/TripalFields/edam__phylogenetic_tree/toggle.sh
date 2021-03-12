#!/bin/bash
set -e
set -x
if [ -e edam__phylogenetic_tree_widget.inc.DEBUG ]; then
  mv edam__phylogenetic_tree.inc{,.OKAY}
  mv edam__phylogenetic_tree.inc{.DEBUG,}
  mv edam__phylogenetic_tree_widget.inc{,.OKAY}
  mv edam__phylogenetic_tree_widget.inc{.DEBUG,}
  mv edam__phylogenetic_tree_formatter.inc{,.OKAY}
  mv edam__phylogenetic_tree_formatter.inc{.DEBUG,}
else
  mv edam__phylogenetic_tree.inc{,.DEBUG}
  mv edam__phylogenetic_tree.inc{.OKAY,}
  mv edam__phylogenetic_tree_widget.inc{,.DEBUG}
  mv edam__phylogenetic_tree_widget.inc{.OKAY,}
  mv edam__phylogenetic_tree_formatter.inc{,.DEBUG}
  mv edam__phylogenetic_tree_formatter.inc{.OKAY,}
fi
