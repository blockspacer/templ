![templ logo](https://bitbucket.org/noobsaibot/templ/raw/a6fd032ee4f57703fa59cde6eaaefc44569d4b60/logo.png)
# Jinja2 C++

## ausdrücke

nachfolgend ist eine liste der ausdrücke (expressions), die soweit unterstützt werden

### literale

    "tolles wetter"
    'tolles wetter'

zeichenketten werden sowohl mit doppelten, als auch mit einfachen anführungszeichen unterstützt.

    42
    42.0

ganzzahlen und fließkommazahlen werden unterstützt.

    ['europa', 'asien', 'australien']

listen von ausdrücken werden unterstützt. listen können variablen zugewiesen, oder zur 
verwendung in `for` schleifen direkt angegeben werden.

    ('x', 'y')

tupel werden unterstützt. überflüssiges komma nach dem letzten element wird geschluckt.

    ['name': 'adam', 'alter': '30']

dictionaries werden noch nicht in vollem umfang unterstützt.

    true
    false

boolische angaben werden unterstützt.

### mathematische ausdrücke

    3+5*7/2

folgende mathematische operatoren können verwendet werden.

    +
    -
    *
    **
    /
    //
    %

## anweisungen

folgende anweisungen werden derzeit unterstützt.

### if

```jinja2
{% if <bedingung> %}
    <anweisungen>
{% elif <bedingung> %}
    <anweisungen>
{% else %}
    <anweisungen>
{% endif %}
```

### for

```jinja2
{% for <iterator> in <menge> %}
    <anweisungen>
{% else %}
    <anweisungen>
{% endfor %}
```

### block

```jinja2
{% block <name> %}
{% endblock %}
```

### include

```jinja2
{% include "<template>" %}
```

### import

```jinja2
{% import "<template>" as <sym> %}
{% from "<template>" import <sym1> as <alias1> %}
```

### extends

```jinja2
{% extends "<template>" %}
```

### filter

```jinja2
{% filter %}
{% endfilter %}
```

### macro

```jinja2
{% macro <name>(parameter, ...) %}
{% endmarcro %}
```

### raw

```jinja2
{% raw %}
{% endraw %}
```
