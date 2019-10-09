<html>
    <head>
        {% set default_title = "tmpl" %}
        <title>{% block title %}{% endblock %}</title>
    </head>
    <body>
        {% include "header.html" %}
        {% set z = ["a", "b", "c"] %}

        <div class="main">
        {% block main %}
            super-block-dass-du-da-bist
        {% endblock %}
        </div>

        {% include "footer.html" %}
    </body>
</html>
