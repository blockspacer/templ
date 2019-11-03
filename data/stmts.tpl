<div>stmts.tpl</div>
{% set a = 10 %}

{% if 3 <= 5 %}
    {{ "3 <= 5" }}
{% elif 3 > 5 %}
    {{ 5 * 10 }}
{% endif %}

{% raw %}
    {% set foo = "bar" %}
    und dann
{% endraw %}

{% with a = a, b = 3 %}
    {% set x = 5 %}
    x = {{ x }}
    a = {{ a }}
{% endwith %}
x = {{ x }}

{% set b = [1, 2, 3] %}
b[1] = {{ b[1] }}
{% set b[1] = 5 %}
b[1] = {{ b[1] }}

{% set c = [(1, 2), (3, 4)] %}
{% set c[1] = (7, 8) %}
c[1][0] {{ c[1][0] }} {# sollte 7 rauskommen #}

{% set d = "シ个abcd" %}
{{ "{% set d = " }}"シ个abcd"{{ "%}" }} = {{ d }}
{% set d[3] = "个" %}
{{ "{% set d[3] = " }} "个" {{ "%}" }} = {{ d }}

{% for i in 1..10 %}
    {% if i == 2 %}{% continue %}{% endif %}
    {{ i }}
    {% if i == 5 %}{% break %}{% endif %}
{% endfor %}

{% for it in range(stop=7, start=1, step=2) %}
    {{ it }}
{% endfor %}

{% set lorem = lipsum() %}
{{ lorem }}