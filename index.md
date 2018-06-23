---
layout: default
home: true

# Conteúdo da Pagina Inicial
---

{% for post in site.posts %}
<section id="{{post.name}}" class="white-space-160">
  {{ content }}
</section>
{% endfor %}
