CarrotOmics-Module
==================

This is a custom Tripal module used on the CarrotOmics.org site.

It is a work-in-progress, and not ready for general use.

Custom Tripal Fields in this module are:

sio__geographic_position
------------------------

This Tripal field can be used on a germplasm entity page to show geolocation,
altitude, and description. CarrotOmics uses a custom addition to the chado.nd_geolocation
table to include uncertainty values, used when the geolocation is only approximate.

![sio__geographic_position example image](/docs/sio__geographic_position_example.jpg?raw=true "Example image of sio__geographic_position tripal field")

iao__image
----------

This Tripal field can be used on a germplasm entity page to show all images of that
germplasm. This field uses a pager, so a large number of images can be handled.

![iao__image example image](/docs/iao__image_example.jpg?raw=true "Example image of iao__image tripal field")

edam__accession
---------------

This Tripal field can be used on an organism entity page to list all germplasm
accessions for that organism.

![edam__accession example image](/docs/edam__accession_example.jpg?raw=true "Example image of edam__accession tripal field")

edam__phylogenetic_tree
-----------------------

This Tripal field can be used on an analysis entity page to list all phylogenetic
trees that are part of that analysis.

![edam__phylogenetic_tree example image](/docs/edam__phylogenetic_tree_example.jpg?raw=true "Example image of edam__phylogenetic_tree tripal field")

data__jbrowse_coordinates
-------------------------

This Tripal Field is a replacement for the `data__sequence_coordinates` field,
the only difference is that it will include a link to JBrowse.

![data__jbrowse_coordinates example image](/docs/data__jbrowse_coordinates_example.jpg?raw=true "Example image of data__jbrowse_coordinates tripal field")

edam__genetic_map
-----------------

This field provides the inverse of the `main__stock_linker` field, it provides a link to
a Tripal Map (featuremap) page from a stock page. The current implementation supports a
link from a population page to a map using that population. This field uses the
`chado.featuremap_stock` table to obtain the link information.

![edam__genetic_map example image](/docs/edam__genetic_map_example.jpg?raw=true "Example image of edam__genetic_map tripal field")
