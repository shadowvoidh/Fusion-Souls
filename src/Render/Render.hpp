#pragma once
// =============================================================================
//  Renderer.hpp
//  Renderização SFML: personagem, inimigos, HUD, cartas, painel de fusão.
// =============================================================================

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <cmath>
#include "Entity.hpp"
#include "Combat.hpp"

static const float SW = 1024.f;
static const float SH = 720.f;

// ---------------------------------------------------------------------------
//  Utilitários de desenho
// ---------------------------------------------------------------------------
inline void drawBar(sf::RenderWindow& win,
                    float x, float y, float w, float h,
                    float pct, sf::Color corFundo, sf::Color corBarra,
                    sf::Color corBorda = sf::Color(30, 30, 30))
{
    // Fundo
    sf::RectangleShape bg({w, h});
    bg.setPosition(x, y);
    bg.setFillColor(corFundo);
    bg.setOutlineColor(corBorda);
    bg.setOutlineThickness(1.f);
    win.draw(bg);

    // Barra preenchida
    float fill = std::max(0.f, std::min(1.f, pct)) * w;
    if (fill > 0.f) {
        sf::RectangleShape bar({fill, h});
        bar.setPosition(x, y);
        bar.setFillColor(corBarra);
        win.draw(bar);
    }
}

inline void drawText(sf::RenderWindow& win, sf::Font& font,
                     const std::string& str, float x, float y,
                     unsigned size = 14, sf::Color cor = sf::Color::White,
                     bool negrito = false)
{
    sf::Text t(str, font, size);
    t.setPosition(x, y);
    t.setFillColor(cor);
    if (negrito) t.setStyle(sf::Text::Bold);
    win.draw(t);
}

// ---------------------------------------------------------------------------
//  Desenho do jogador (procedural, visual muda com a fusão)
// ---------------------------------------------------------------------------
inline void drawJogador(sf::RenderWindow& win, const Jogador& j, float animT) {
    float sc    = j.escalaVisual;
    float px    = j.pos.x;
    float py    = j.pos.y;
    float walk  = std::sin(animT * 8.f) * 5.f;

    // Shadow
    sf::CircleShape shadow(14.f * sc, 20);
    shadow.setScale(1.f, 0.35f);
    shadow.setOrigin(14.f * sc, 14.f * sc);
    shadow.setPosition(px, py + 22.f * sc);
    shadow.setFillColor(sf::Color(0, 0, 0, 60));
    win.draw(shadow);

    // Asas (Dracônico >= 40%)
    if (j.temAsas) {
        sf::Color corAsa = j.corPrimaria;
        corAsa.a = 180;
        for (int lado = -1; lado <= 1; lado += 2) {
            sf::VertexArray asa(sf::Triangles, 3);
            asa[0].position = {px + lado * 8.f * sc, py - 10.f * sc};
            asa[1].position = {px + lado * 36.f * sc, py - 30.f * sc + std::sin(animT * 3.f) * 6.f};
            asa[2].position = {px + lado * 30.f * sc, py + 10.f * sc};
            asa[0].color = corAsa;
            asa[1].color = sf::Color(corAsa.r, corAsa.g, corAsa.b, 80);
            asa[2].color = corAsa;
            win.draw(asa);
        }
    }

    // Legs / boots
    sf::RectangleShape perna({6.f * sc, 14.f * sc});
    perna.setOrigin(3.f * sc, 0.f);
    perna.setFillColor(sf::Color(30, 30, 40));
    perna.setPosition(px - 5.f * sc + walk, py + 10.f * sc);  win.draw(perna);
    perna.setPosition(px + 5.f * sc - walk, py + 10.f * sc);  win.draw(perna);

    // Corpo principal
    sf::RectangleShape corpo({20.f * sc, 22.f * sc});
    corpo.setOrigin(10.f * sc, 0.f);
    corpo.setFillColor(j.corPrimaria);
    corpo.setPosition(px, py - 10.f * sc);
    win.draw(corpo);

    // Detalhe do corpo (linhagem secundária)
    sf::RectangleShape detalhe({4.f * sc, 22.f * sc});
    detalhe.setOrigin(2.f * sc, 0.f);
    detalhe.setFillColor(j.corSecundaria);
    detalhe.setPosition(px, py - 10.f * sc);
    win.draw(detalhe);

    // Braços
    sf::RectangleShape braco({7.f * sc, 16.f * sc});
    braco.setOrigin(3.5f * sc, 0.f);
    braco.setFillColor(j.corPrimaria);
    braco.setPosition(px - 13.f * sc, py - 8.f * sc);  win.draw(braco);
    braco.setPosition(px + 13.f * sc, py - 8.f * sc);  win.draw(braco);

    // Escudo (Aegisknight >= 35%)
    if (j.temEscudo) {
        sf::CircleShape escudo(18.f * sc, 6);
        escudo.setOrigin(18.f * sc, 18.f * sc);
        escudo.setPosition(px - 20.f * sc, py - 2.f * sc);
        escudo.setFillColor(sf::Color(80, 100, 200, 160));
        escudo.setOutlineColor(sf::Color(180, 200, 255));
        escudo.setOutlineThickness(2.f);
        win.draw(escudo);
    }

    // Cabeça
    sf::CircleShape cabeca(10.f * sc, 16);
    cabeca.setOrigin(10.f * sc, 10.f * sc);
    cabeca.setFillColor(sf::Color(235, 195, 150));
    cabeca.setPosition(px, py - 20.f * sc);
    win.draw(cabeca);

    // Olhos (direção)
    sf::CircleShape olho(2.f * sc, 8);
    olho.setOrigin(2.f * sc, 2.f * sc);
    olho.setFillColor(sf::Color(20, 20, 30));
    olho.setPosition(px - 4.f * sc + j.dir.x * 3.f, py - 22.f * sc + j.dir.y * 2.f);
    win.draw(olho);
    olho.setPosition(px + 4.f * sc + j.dir.x * 3.f, py - 22.f * sc + j.dir.y * 2.f);
    win.draw(olho);

    // Efeito de iframe (piscada de invulnerabilidade)
    if (j.iframeTimer > 0.f) {
        sf::CircleShape iframe(22.f * sc, 20);
        iframe.setOrigin(22.f * sc, 22.f * sc);
        iframe.setPosition(px, py - 5.f * sc);
        iframe.setFillColor(sf::Color::Transparent);
        iframe.setOutlineColor(sf::Color(255, 255, 255, 150));
        iframe.setOutlineThickness(2.f);
        win.draw(iframe);
    }

    // Espíritos do Zexcromante
    for (int i = 0; i < j.numEspiritos; ++i) {
        float ang  = animT * 1.5f + i * (3.14159f * 2.f / 3.f);
        float ox   = std::cos(ang) * 40.f * sc;
        float oy   = std::sin(ang) * 30.f * sc;
        sf::CircleShape esp(8.f, 8);
        esp.setOrigin(8.f, 8.f);
        esp.setPosition(px + ox, py - 10.f * sc + oy);
        esp.setFillColor(sf::Color(180, 140, 255, 180));
        esp.setOutlineColor(sf::Color(220, 200, 255));
        esp.setOutlineThickness(1.f);
        win.draw(esp);
        // Espada do espírito
        sf::RectangleShape espada({2.f, 14.f});
        espada.setOrigin(1.f, 7.f);
        espada.setPosition(px + ox, py - 10.f * sc + oy);
        espada.setFillColor(sf::Color(200, 220, 255, 200));
        espada.setRotation(ang * 57.3f);
        win.draw(espada);
    }
}

// ---------------------------------------------------------------------------
//  Desenho de inimigo/chefe
// ---------------------------------------------------------------------------
inline void drawInimigo(sf::RenderWindow& win, const Inimigo& e, float animT) {
    if (!e.vivo) return;

    float px = e.pos.x;
    float py = e.pos.y;

    // Cor base vinda da linhagem
    sf::Color cor = sf::Color(180, 60, 60);
    if (!e.fases.empty() && e.faseAtual < (int)e.fases.size())
        cor = e.fases[e.faseAtual].corFase;
    else if (e.eChefe)
        cor = sf::Color(200, 50, 30);

    // Piscada de dano
    bool piscando = (e.iframeTimer > 0.f);
    if (piscando) cor = sf::Color::White;

    float sc = e.eChefe ? 1.6f : (e.eSubChefe ? 1.3f : 1.0f);

    // Shadow
    sf::CircleShape shadow(16.f * sc, 20);
    shadow.setScale(1.f, 0.35f);
    shadow.setOrigin(16.f * sc, 16.f * sc);
    shadow.setPosition(px, py + 26.f * sc);
    shadow.setFillColor(sf::Color(0, 0, 0, 70));
    win.draw(shadow);

    // Corpo
    sf::RectangleShape corpo({22.f * sc, 28.f * sc});
    corpo.setOrigin(11.f * sc, 0.f);
    corpo.setFillColor(cor);
    corpo.setPosition(px, py - 12.f * sc);
    win.draw(corpo);

    // Braços
    sf::RectangleShape braco({8.f * sc, 18.f * sc});
    braco.setOrigin(4.f * sc, 0.f);
    braco.setFillColor(sf::Color(cor.r * 0.8f, cor.g * 0.8f, cor.b * 0.8f));
    braco.setPosition(px - 15.f * sc, py - 10.f * sc);  win.draw(braco);
    braco.setPosition(px + 15.f * sc, py - 10.f * sc);  win.draw(braco);

    // Cabeça
    sf::CircleShape cabeca(12.f * sc, 16);
    cabeca.setOrigin(12.f * sc, 12.f * sc);
    cabeca.setFillColor(sf::Color(cor.r * 0.9f, cor.g * 0.9f, cor.b * 0.9f));
    cabeca.setPosition(px, py - 24.f * sc);
    win.draw(cabeca);

    // Olhos vermelhos (chefes)
    if (e.eChefe || e.eSubChefe) {
        sf::CircleShape olho(3.f * sc, 8);
        olho.setOrigin(3.f * sc, 3.f * sc);
        olho.setFillColor(sf::Color(255, 50, 0));
        olho.setPosition(px - 5.f * sc, py - 26.f * sc);  win.draw(olho);
        olho.setPosition(px + 5.f * sc, py - 26.f * sc);  win.draw(olho);
    }

    // Indicator de fase atual
    if (e.eChefe && !e.fases.empty()) {
        float indicX = px - 20.f;
        for (int i = 0; i < (int)e.fases.size(); ++i) {
            sf::CircleShape dot(4.f, 8);
            dot.setOrigin(4.f, 4.f);
            dot.setPosition(indicX + i * 12.f, py - 50.f * sc);
            dot.setFillColor(i <= e.faseAtual ?
                sf::Color(255, 220, 0) : sf::Color(60, 60, 60));
            win.draw(dot);
        }
    }

    // HP Bar do inimigo (flutuante)
    float barW = 60.f * sc;
    drawBar(win,
        px - barW * 0.5f, py - 55.f * sc, barW, 6.f * sc,
        e.pctHP(),
        sf::Color(60, 20, 20),
        e.pctHP() > 0.5f ? sf::Color(60, 200, 60) :
        e.pctHP() > 0.25f ? sf::Color(200, 200, 0) : sf::Color(220, 40, 0));

    // Nome
}

// ---------------------------------------------------------------------------
//  Projéteis e áreas
// ---------------------------------------------------------------------------
inline void drawProjeteis(sf::RenderWindow& win, const std::vector<Projetil>& projeteis) {
    for (const auto& p : projeteis) {
        if (!p.ativo) continue;
        sf::CircleShape c(p.raio, 8);
        c.setOrigin(p.raio, p.raio);
        c.setPosition(p.pos);
        c.setFillColor(p.cor);
        // Glow (segundo círculo maior translúcido)
        sf::CircleShape glow(p.raio * 2.f, 12);
        glow.setOrigin(p.raio * 2.f, p.raio * 2.f);
        glow.setPosition(p.pos);
        glow.setFillColor(sf::Color(p.cor.r, p.cor.g, p.cor.b, 60));
        win.draw(glow);
        win.draw(c);
    }
}

inline void drawAreas(sf::RenderWindow& win, const std::vector<AreaEfeito>& areas) {
    for (const auto& a : areas) {
        if (!a.ativo) continue;
        sf::CircleShape c(a.raio, 24);
        c.setOrigin(a.raio, a.raio);
        c.setPosition(a.pos);
        c.setFillColor(a.cor);
        c.setOutlineColor(sf::Color(a.cor.r, a.cor.g, a.cor.b, 180));
        c.setOutlineThickness(2.f);
        win.draw(c);
    }
}

// ---------------------------------------------------------------------------
//  HUD: barras, cartas, fusão
// ---------------------------------------------------------------------------
inline void drawHUD(sf::RenderWindow& win, sf::Font& font, const Jogador& j, float animT) {
    // ── Painel de recursos (canto inferior esquerdo) ──────────────────
    float bx = 12.f, by = SH - 90.f;
    float bw = 220.f, bh = 14.f;

    // HP
    sf::Color hpCor = j.pctHP() > 0.5f ? sf::Color(60, 210, 60) :
                      j.pctHP() > 0.25f ? sf::Color(220, 200, 0) : sf::Color(220, 40, 40);
    drawText(win, font, "HP", bx, by - 16, 12, sf::Color(200, 200, 200));
    drawBar(win, bx, by, bw, bh, j.pctHP(), sf::Color(40, 20, 20), hpCor);
    drawText(win, font,
        std::to_string((int)j.hpAtual) + "/" + std::to_string((int)j.stats.hp),
        bx + 4, by + 1, 10, sf::Color::White);

    // Escudo
    if (j.shieldAtual > 0.f) {
        drawBar(win, bx, by - 4, bw * (j.shieldAtual / std::max(1.f, j.stats.shieldHP)),
                4.f, 1.f, sf::Color::Transparent, sf::Color(100, 150, 255));
    }

    // Mana
    by += bh + 5.f;
    drawText(win, font, "MP", bx, by - 16, 12, sf::Color(180, 160, 255));
    drawBar(win, bx, by, bw, bh, j.pctMana(), sf::Color(20, 20, 60), sf::Color(100, 80, 230));
    drawText(win, font,
        std::to_string((int)j.manaAtual) + "/" + std::to_string((int)j.stats.mana),
        bx + 4, by + 1, 10, sf::Color::White);

    // Stamina
    by += bh + 5.f;
    sf::Color stamCor = j.staminaAtual / std::max(1.f, j.stats.stamina) > 0.3f ?
                        sf::Color(240, 200, 60) : sf::Color(180, 60, 30);
    drawText(win, font, "ST", bx, by - 16, 12, sf::Color(240, 200, 60));
    drawBar(win, bx, by, bw, bh, j.staminaAtual / j.stats.stamina,
            sf::Color(50, 40, 20), stamCor);

    // Fúria Berzerker
    if (j.cargasFuria > 0) {
        by += bh + 5.f;
        drawText(win, font, "FURIA", bx, by - 16, 12, sf::Color(255, 80, 0));
        for (int i = 0; i < j.maxCargasFuria; ++i) {
            sf::RectangleShape charge({18.f, 10.f});
            charge.setPosition(bx + i * 21.f, by);
            charge.setFillColor(i < j.cargasFuria ?
                sf::Color(255, 80 + i * 15, 0) : sf::Color(60, 30, 20));
            charge.setOutlineColor(sf::Color(100, 40, 10));
            charge.setOutlineThickness(1.f);
            win.draw(charge);
        }
    }

    // ── Fusion Souls: minipainel (canto superior esquerdo) ────────────
    float fx = 10.f, fy = 10.f;
    sf::RectangleShape fusPanel({200.f, 130.f});
    fusPanel.setPosition(fx, fy);
    fusPanel.setFillColor(sf::Color(0, 0, 0, 180));
    fusPanel.setOutlineColor(sf::Color(80, 60, 120));
    fusPanel.setOutlineThickness(1.f);
    win.draw(fusPanel);

    drawText(win, font, "FUSION SOULS", fx + 8, fy + 6, 11,
             sf::Color(200, 180, 255), true);
    drawText(win, font, nomeFusao(j.fusao), fx + 8, fy + 20, 12,
             sf::Color(255, 220, 100));

    auto top = j.fusao.top(4);
    for (int i = 0; i < (int)top.size(); ++i) {
        auto& [lin, pct] = top[i];
        float ly = fy + 38.f + i * 22.f;
        std::string label = nomeLinhagem(lin);
        drawText(win, font, label, fx + 8, ly, 10, sf::Color(200, 200, 200));
        drawBar(win, fx + 85, ly + 2, 100.f, 10.f, pct,
            sf::Color(30, 30, 30),
            lin == Linhagem::Dracconico  ? sf::Color(220, 80, 30) :
            lin == Linhagem::Berzerker   ? sf::Color(160, 30, 30) :
            lin == Linhagem::Zexcromante ? sf::Color(130, 60, 200) :
            lin == Linhagem::SniperVision? sf::Color(50, 130, 180) :
            lin == Linhagem::Chronoguard ? sf::Color(100, 200, 220) :
            lin == Linhagem::Toxishaman  ? sf::Color(80, 180, 60) :
            lin == Linhagem::Aegisknight ? sf::Color(180, 180, 200) :
            sf::Color(180, 160, 120));
        drawText(win, font,
            std::to_string((int)(pct * 100)) + "%",
            fx + 188, ly, 10, sf::Color(220, 220, 180));
    }

    // ── Cartas ativas (barra inferior) ───────────────────────────────
    float cx = SW * 0.5f - 4 * 62.f, cy = SH - 76.f;
    auto usaveis = const_cast<DeckAtivo&>(j.deck).cartasUsaveis(j.fusao);

    // Mostra até 8 cartas
    int shown = std::min((int)usaveis.size(), 8);
    for (int i = 0; i < shown; ++i) {
        const Carta* c = usaveis[i];
        float cx2 = cx + i * 62.f;
        bool emCD = c->cooldownRestante > 0.f;

        // Fundo da carta
        sf::RectangleShape card({56.f, 68.f});
        card.setPosition(cx2, cy);
        card.setFillColor(emCD ? sf::Color(30, 30, 30, 200) : sf::Color(20, 20, 40, 220));
        card.setOutlineColor(
            c->linhagem == Linhagem::Dracconico  ? sf::Color(220, 80, 0) :
            c->linhagem == Linhagem::Berzerker   ? sf::Color(160, 30, 0) :
            c->linhagem == Linhagem::Zexcromante ? sf::Color(130, 60, 200) :
            c->linhagem == Linhagem::SniperVision? sf::Color(50, 130, 200) :
            c->linhagem == Linhagem::Chronoguard ? sf::Color(0, 200, 220) :
            c->linhagem == Linhagem::Toxishaman  ? sf::Color(60, 200, 30) :
            c->linhagem == Linhagem::Aegisknight ? sf::Color(150, 170, 220) :
            sf::Color(150, 150, 150));
        card.setOutlineThickness(emCD ? 1.f : 2.f);
        win.draw(card);

        // Nome (truncado)
        std::string nome = c->nome;
        if (nome.size() > 9) nome = nome.substr(0, 8) + ".";
        drawText(win, font, nome, cx2 + 2, cy + 4, 8, sf::Color(220, 220, 220));

        // Ícone por tipo
        std::string icon =
            c->efeito == EfeitoCarta::Projetil     ? ">>>" :
            c->efeito == EfeitoCarta::AoE           ? "***" :
            c->efeito == EfeitoCarta::DashEvasao    ? "~~~" :
            c->efeito == EfeitoCarta::Cura          ? "+++" :
            c->efeito == EfeitoCarta::Escudo        ? "[X]" :
            c->efeito == EfeitoCarta::SlowTempo     ? "@@@" :
            c->efeito == EfeitoCarta::InvocarEspirito? "S S" :
            c->efeito == EfeitoCarta::RebobinarVida ? "<<<" :
            c->efeito == EfeitoCarta::CargaFrenesi  ? "!!!" : "---";
        drawText(win, font, icon, cx2 + 14, cy + 20, 14,
                 emCD ? sf::Color(80, 80, 80) : sf::Color(255, 220, 100));

        // Cooldown overlay
        if (emCD) {
            float cdPct = c->cooldownRestante / c->custoCooldown;
            sf::RectangleShape cdOver({56.f, 68.f * cdPct});
            cdOver.setPosition(cx2, cy + 68.f * (1.f - cdPct));
            cdOver.setFillColor(sf::Color(0, 0, 0, 160));
            win.draw(cdOver);
            std::string cdStr = std::to_string((int)c->cooldownRestante + 1) + "s";
            drawText(win, font, cdStr, cx2 + 18, cy + 30, 12, sf::Color(200, 200, 200));
        }

        // Custo de mana/stamina
        if (c->custoMana > 0)
            drawText(win, font, "M:" + std::to_string((int)c->custoMana),
                     cx2 + 2, cy + 50, 8, sf::Color(150, 130, 255));
        if (c->custoStamina > 0)
            drawText(win, font, "S:" + std::to_string((int)c->custoStamina),
                     cx2 + 2, cy + 60, 8, sf::Color(220, 200, 60));

        // Número do slot
        drawText(win, font, std::to_string(i + 1),
                 cx2 + 44, cy + 54, 10, sf::Color(120, 120, 120));
    }

    // ── Nível e XP (canto superior direito) ──────────────────────────
    float lx = SW - 160.f, ly = 10.f;
    drawText(win, font, "LVL " + std::to_string(j.nivel),
             lx, ly, 16, sf::Color(255, 220, 100), true);
    drawBar(win, lx, ly + 20, 140.f, 8.f,
            (float)j.experiencia / j.xpParaProxNivel,
            sf::Color(30, 30, 60), sf::Color(80, 120, 255));
    drawText(win, font,
        "XP " + std::to_string(j.experiencia) + "/" + std::to_string(j.xpParaProxNivel),
        lx, ly + 30, 10, sf::Color(160, 160, 200));

    // Status effects ativos
    float sx = SW - 160.f, sy = 60.f;
    for (const auto& s : j.statusAtivos) {
        if (!s.ativo) continue;
        std::string nome =
            s.tipo == TipoStatus::Veneno     ? "VENENO" :
            s.tipo == TipoStatus::Queimadura ? "QUEIMADURA" :
            s.tipo == TipoStatus::Paralisia  ? "PARALISIA" :
            s.tipo == TipoStatus::Lento      ? "LENTO" :
            s.tipo == TipoStatus::Atordoado  ? "ATORDOADO" : "";
        sf::Color cor =
            s.tipo == TipoStatus::Veneno     ? sf::Color(80, 220, 60) :
            s.tipo == TipoStatus::Queimadura ? sf::Color(255, 120, 0) :
            s.tipo == TipoStatus::Lento      ? sf::Color(60, 180, 255) : sf::Color::White;
        drawText(win, font, nome + " " + std::to_string((int)s.duracao + 1) + "s",
                 sx, sy, 11, cor);
        sy += 16.f;
    }
}

// ---------------------------------------------------------------------------
//  Tela de Fusion Souls (menu de reconfiguração)
// ---------------------------------------------------------------------------
inline void drawFusionMenu(sf::RenderWindow& win, sf::Font& font,
                            const FusionSouls& f, const Stats& stats)
{
    // Fundo semi-transparente
    sf::RectangleShape bg({SW, SH});
    bg.setFillColor(sf::Color(0, 0, 0, 210));
    win.draw(bg);

    drawText(win, font, "== FUSION SOULS ==", SW * 0.5f - 100, 30, 22,
             sf::Color(220, 180, 255), true);
    drawText(win, font, "Configure sua linhagem (Enter para fechar)",
             SW * 0.5f - 150, 60, 12, sf::Color(180, 180, 180));

    // Barras de linhagem
    float ly = 100.f;
    for (int i = 0; i < static_cast<int>(Linhagem::COUNT) - 1; ++i) {
        Linhagem l = static_cast<Linhagem>(i);
        float pct  = f.get(l);
        std::string nome = nomeLinhagem(l);

        drawText(win, font, nome, 80, ly, 14, sf::Color(220, 220, 220));
        drawBar(win, 240, ly + 2, 300, 16, pct,
                sf::Color(40, 40, 50), sf::Color(150, 100, 220));
        drawText(win, font, std::to_string((int)(pct * 100)) + "%",
                 550, ly + 2, 13, sf::Color(255, 220, 100));
        ly += 28.f;
    }

    // Stats calculados
    float sx2 = 700.f, sy2 = 100.f;
    drawText(win, font, "== ATRIBUTOS ==", sx2, sy2, 14, sf::Color(200, 200, 255), true);
    sy2 += 24;
    auto line = [&](const std::string& label, float val) {
        drawText(win, font, label + ": " + std::to_string((int)val), sx2, sy2, 12);
        sy2 += 18;
    };
    line("HP",        stats.hp);
    line("ATK",       stats.atk);
    line("ATK MAG",   stats.atkMagico);
    line("DEF",       stats.def);
    line("DEF MAG",   stats.defMagica);
    line("SPD",       stats.spd);
    line("MANA",      stats.mana);
    line("STAMINA",   stats.stamina);
    line("CRIT%",     stats.critChance * 100);
    line("CRIT MULT", stats.critMult);
    if (stats.poisonDmg  > 0) line("VENENO/s", stats.poisonDmg);
    if (stats.shieldHP   > 0) line("ESCUDO",   stats.shieldHP);
    if (stats.timeSlowPct> 0) line("SLOW%",    stats.timeSlowPct * 100);
    if (stats.spiritCount> 0) line("ESPIRITOS",(int)stats.spiritCount);
    if (stats.fireResist > 0) line("RESIST.FOGO",stats.fireResist * 100);

    // Passivas ativas
    sy2 += 10;
    drawText(win, font, "== PASSIVAS ATIVAS ==", sx2, sy2, 13,
             sf::Color(255, 220, 100), true);
    sy2 += 20;
    auto ativos = passivasAtivas(f);
    for (auto& p : ativos) {
        drawText(win, font, "* " + p.nome, sx2, sy2, 11, sf::Color(200, 255, 180));
        sy2 += 14;
    }
    if (ativos.empty())
        drawText(win, font, "(nenhuma ativa)", sx2, sy2, 11, sf::Color(120, 120, 120));

    drawText(win, font, nomeFusao(f),
             SW * 0.5f - 80, SH - 50, 16, sf::Color(255, 200, 80), true);
}

// ---------------------------------------------------------------------------
//  Números de dano flutuantes
// ---------------------------------------------------------------------------
struct DanoFlutuante {
    sf::Vector2f pos;
    float        valor   = 0.f;
    float        vida    = 1.2f;
    bool         critico = false;
    bool         cura    = false;
    sf::Color    cor     = sf::Color::White;
};

inline void updateDrawDanos(sf::RenderWindow& win, sf::Font& font,
                             std::vector<DanoFlutuante>& danos, float dt)
{
    for (auto& d : danos) {
        d.vida -= dt;
        d.pos.y -= 30.f * dt;  // Sobe

        if (d.vida > 0.f) {
            sf::Uint8 alpha = sf::Uint8(std::min(255.f, d.vida / 1.2f * 255.f));
            std::string str = (d.cura ? "+" : "-") + std::to_string((int)d.valor);
            if (d.critico) str += "!";
            sf::Color cor(d.cor.r, d.cor.g, d.cor.b, alpha);
            drawText(win, font, str, d.pos.x, d.pos.y,
                     d.critico ? 18 : 14, cor, d.critico);
        }
    }
    danos.erase(
        std::remove_if(danos.begin(), danos.end(),
            [](const DanoFlutuante& d){ return d.vida <= 0.f; }),
        danos.end());
}