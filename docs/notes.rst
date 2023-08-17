Notes
=======

Developing the documentation
*****************************
When ready to compile the sphinx documentation, run the following commands

.. code-block:: bash

    $ cd docs
    $ make html

I would recommend downloading the VSCode extension `Live Server` to preview HTML pages in real time.
This extension works for SSH remotes as well.
If using an SSH remote, you can access the webpage thorugh http://<hostname>:5500/docs/_build/html/index.html (or whatever your port is).

This extension is great because as you update the documentation through the command ``make html``, it automatically updates the contents on your live preview without refreshing.

Adding new pages
*****************
Simply create a new ``.rst`` file in the `docs` folder and add the name of that file to ``index.rst`` under `toctree`.

Some Useful Tools
**********************
We can also insert pictures in our text. We place the photos in the `_static` directory under `docs`.

.. image:: _static/lenna.png
  :width: 200
  :alt: Lenna Test Image

We can show inline code using two grave markers on each side of a block. ``This is inline code``
Additionally, code blocks are useful too [#f1]_.

.. code-block:: bash

    $ pwd
    <path_to_systemc>/systemc-2.3.4
    $ mkdir build
    $ cd build
    $ ccmake .. 

References
****************

I found this guide to be good for formatting different things in `.rst` format (reStructuredText).
https://sublime-and-sphinx-guide.readthedocs.io/en/latest/index.html

.. rubric:: Footnotes

.. [#f1] A. Boutros, E. Nurvitadhi, and V. Betz, "RAD-Sim: Rapid Architecture Exploration for Novel Reconfigurable Acceleration Devices", International Conference on Field Programmable Logic and Applications (FPL), 2022