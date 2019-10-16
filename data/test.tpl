{% extends "main.tpl" %}

{% import "macros.tpl" as macros %}
{% from "macros.tpl" import test1 as test_1, test2 %}

{% block title %}
    test - {{ default_title }}
{% endblock %}

{% block main %}
    <h2>super()</h2>
    {{ super() }}

    <h2>templ vars</h2>
    {{ name }}

    <h2>import macros</h2>
    {{ macros.test1() }}

    <h2>import variablen</h2>
    {{ macros.macro_name }}

    <h2>from import macro</h2>
    {{ test_1() }}
    {{ test2() }}

    <h2>macro</h2>
    {{ macros.printf(arg='10, 5', format="%d = %d") }}
    {{ macros.printf("zeugs %s zum formatieren") }}

    <h2>for mit zeichenketten</h2>
    {% for it in "d".."i" %}
        <div>{{ it }}</div>
    {% endfor %}

    <h2>set anweisung</h2>
    {% set a = "test" %}
    <div class="bla">{{ a }}</div>

    <h2>if anweisung</h2>
    {% set b = 2 %}
    {% if b is eq 2 %}
        {#{ user.name if b is eq 2 else "rumpelstilzchen" }#}
    {% endif %}

    <h2>array literal</h2>
    {% for it in z %}
        <div>{{ it }}</div>
    {% endfor %}

    <h2>range</h2>
    {% for it in 0..5 %}
        <div>{{ it }}</div>
    {% else %}
        <div>menge ist leer</div>
    {% endfor %}

    {% for it in 0..0 %}
        <div>{{ it }}</div>
    {% else %}
        <div>menge ist leer</div>
    {% endfor %}
{% endblock main %}

{% block foo %}
    <div>hier kommt foo hin</div>
{% endblock %}
