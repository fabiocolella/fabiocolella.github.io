---
layout: default
home: true

# Conteúdo da Pagina Inicial
---

{% for post in site.posts %}
<section id="{{post.name}}" class="white-space-160">
  <div class="container">
    <div class="row home1-about text-center">
      <div class="col-12 col-md-8 offset-md-2 home1-about__info">
        {{ content }}
      </div>
    </div>
  </div>
</section>
{% endfor %}
