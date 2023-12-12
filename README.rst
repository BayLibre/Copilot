Copilot
=======

Hardware files are in the ``Copilot_Lite`` folder.

The user documentation is available in the ``docs`` folder.
The generated HTML pages can be viewed `here <https://baylibre.pages.baylibre.com/copilot/copilot/>`_

Doc build instructions
----------------------

Using python3, do:

.. code-block:: console

   $ pip3 install -U sphinx sphinx-rtd-theme sphinx-prompt


Then rebuild with:

.. code-block:: console

   $ cd doc
   $ make html

The build output is located in ``doc/build/html/``. Open it with:

.. code-block:: console

   $ xdg-open doc/build/html/index.html


Tips and tricks
---------------

To find a reference to another section, use:

.. code-block:: console

   $ python -m sphinx.ext.intersphinx doc/build/html/objects.inv
