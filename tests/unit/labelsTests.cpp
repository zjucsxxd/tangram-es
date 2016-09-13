#include "catch.hpp"
#include "tangram.h"
#include "platform.h"
#include "scene/scene.h"
#include "style/style.h"
#include "style/textStyle.h"
#include "style/pointStyle.h"
#include "labels/labels.h"
#include "labels/textLabel.h"
#include "labels/textLabels.h"
#include "gl/dynamicQuadMesh.h"

#include "view/view.h"
#include "tile/tile.h"

#include <memory>

namespace Tangram {

TextStyle dummyStyle("textStyle", nullptr);
TextLabels dummy(dummyStyle);


std::unique_ptr<TextLabel> makeLabel(Label::Transform _transform, Label::Type _type,
                                     std::string id, float priority = 0,
                                     glm::vec2 dim = {10, 10},
                                     LabelProperty::Anchor anchor = LabelProperty::Anchor::center,
                                     bool required = false) {
    Label::Options options;
    options.required = required;
    options.priority = priority;
    options.offset = {0.0f, 0.0f};
    options.properties = std::make_shared<Properties>();
    options.properties->set("id", id);
    options.interactive = true;
    options.collide = true;
    options.anchors.anchor[0] = anchor;
    options.anchors.count = 1;

    return std::unique_ptr<TextLabel>(new TextLabel(_transform, _type, options,
                                                    {}, dim, dummy, {},
                                                    TextLabelProperty::Align::none));
}

std::unique_ptr<SpriteLabel> makeSpriteLabel(Label::Transform _transform,
                                           std::string id, float priority = 0,
                                           LabelProperty::Anchor anchor = LabelProperty::Anchor::center) {
    static const PointStyle dummyStyle("pointStyle", nullptr);

    static SpriteLabels dummy(dummyStyle);

    Label::Options options;
    options.priority = priority;
    options.offset = {0.0f, 0.0f};
    options.properties = std::make_shared<Properties>();
    options.properties->set("id", id);
    options.interactive = true;
    options.collide = true;
    options.anchors.anchor[0] = anchor;
    options.anchors.count = 1;

    return std::unique_ptr<SpriteLabel>(new SpriteLabel(_transform, {10, 10}, options,
                                                        1, dummy, 0));
}

TextLabel makeLabelWithAnchorFallbacks(Label::Transform _transform, glm::vec2 _offset = {0, 0}) {
    Label::Options options;

    // options.anchors.anchor[0] = LabelProperty::Anchor::center;
    options.anchors.anchor[0] = LabelProperty::Anchor::right;
    options.anchors.anchor[1] = LabelProperty::Anchor::bottom;
    options.anchors.anchor[2] = LabelProperty::Anchor::left;
    options.anchors.anchor[3] = LabelProperty::Anchor::top;
    options.anchors.count = 4;

    options.offset = _offset;

    TextRange textRanges;

    return TextLabel(_transform, Label::Type::point, options,
            {}, {10, 10}, dummy, textRanges, TextLabelProperty::Align::none);
}

TEST_CASE("Test getFeaturesAtPoint", "[Labels][FeaturePicking]") {
    std::unique_ptr<Labels> labels(new Labels());

    View view(256, 256);
    view.setPosition(0, 0);
    view.setZoom(0);
    view.update(false);

    struct TestLabelMesh : public LabelSet {
        void addLabel(std::unique_ptr<Label> _label) { m_labels.push_back(std::move(_label)); }
    };

    auto labelMesh = std::unique_ptr<TestLabelMesh>(new TestLabelMesh());
    auto textStyle = std::unique_ptr<TextStyle>(new TextStyle("test", nullptr, false));
    textStyle->setID(0);

    labelMesh->addLabel(makeLabel(glm::vec2{.5f,.5f}, Label::Type::point, "0"));
    labelMesh->addLabel(makeLabel(glm::vec2{1,0}, Label::Type::point, "1"));
    labelMesh->addLabel(makeLabel(glm::vec2{1,1}, Label::Type::point, "2"));

    std::shared_ptr<Tile> tile(new Tile({0,0,0}, view.getMapProjection()));
    tile->initGeometry(1);
    tile->setMesh(*textStyle.get(), std::move(labelMesh));
    tile->update(0, view);

    std::vector<std::unique_ptr<Style>> styles;
    styles.push_back(std::move(textStyle));

    std::vector<std::shared_ptr<Tile>> tiles;

    tiles.push_back(tile);
    {
        auto& items = labels->getFeaturesAtPoint(view, 0, styles, tiles, 128, 128, false);
        REQUIRE(items.size() == 1);
        REQUIRE(items[0].properties->getString("id") == "0");
    }
    {
        auto& items = labels->getFeaturesAtPoint(view, 0, styles, tiles, 256, 256, false);
        REQUIRE(items.size() == 1);
        REQUIRE(items[0].properties->getString("id") == "1");
    }

    {
        auto& items = labels->getFeaturesAtPoint(view, 0, styles, tiles, 256, 0, false);
        REQUIRE(items.size() == 1);
        REQUIRE(items[0].properties->getString("id") == "2");
    }
}

TEST_CASE( "Test anchor fallback behavior", "[Labels][AnchorFallback]" ) {

    View view(256, 256);
    view.setPosition(0, 0);
    view.setZoom(0);
    view.update(false);

    Tile tile({0,0,0}, view.getMapProjection());
    tile.update(0, view);

    glm::vec2 screenSize = glm::vec2(view.getWidth(), view.getHeight());

    class TestLabels : public Labels {
    public:
        TestLabels(View& _v) {
            m_isect2d.resize({1, 1}, {_v.getWidth(), _v.getHeight()});
        }
        void addLabel(Label* _l, Tile* _t) { m_labels.push_back({_l, _t, false}); }
        void run(View& _v) { handleOcclusions(_v); }
        void clear() { m_labels.clear(); }
    };

    TestLabels labels(view);

    {
        TextLabel l1 = makeLabelWithAnchorFallbacks(glm::vec2{0.5,0.5});
        l1.update(tile.mvp(), screenSize, true);
        labels.addLabel(&l1, &tile);

        TextLabel l2 = makeLabelWithAnchorFallbacks(glm::vec2{0.5,0.5});
        l2.update(tile.mvp(), screenSize, true);
        labels.addLabel(&l2, &tile);

        labels.run(view);
        REQUIRE(l1.isOccluded() == false);
        REQUIRE(l2.isOccluded() == true);

        REQUIRE(l1.anchorType() == LabelProperty::Anchor::right);
        REQUIRE(l2.anchorType() == LabelProperty::Anchor::right);
    }
    labels.clear();

    {
        TextLabel l1 = makeLabelWithAnchorFallbacks(glm::vec2{0.5,0.5});
        l1.update(tile.mvp(), screenSize, true);
        labels.addLabel(&l1, &tile);

        // Second label is one pixel left of L1
        TextLabel l2 = makeLabelWithAnchorFallbacks(glm::vec2{0.5 - 1./256,0.5});
        l2.update(tile.mvp(), screenSize, true);
        labels.addLabel(&l2, &tile);

        labels.run(view);
        // l1.print();
        // l2.print();
        REQUIRE(l1.isOccluded() == false);
        REQUIRE(l2.isOccluded() == false);

        REQUIRE(l1.anchorType() == LabelProperty::Anchor::right);
        // Check that left-of anchor fallback is used
        REQUIRE(l2.anchorType() == LabelProperty::Anchor::left);
    }
    labels.clear();

    {
        TextLabel l1 = makeLabelWithAnchorFallbacks(glm::vec2{0.5,0.5});
        l1.update(tile.mvp(), screenSize, true);
        labels.addLabel(&l1, &tile);

        // Second label is 10 pixel top of L1
        TextLabel l2 = makeLabelWithAnchorFallbacks(glm::vec2{0.5,0.5 + 10./256});
        l2.update(tile.mvp(), screenSize, true);
        labels.addLabel(&l2, &tile);

        labels.run(view);
        REQUIRE(l1.isOccluded() == false);
        REQUIRE(l2.isOccluded() == false);

        REQUIRE(l1.anchorType() == LabelProperty::Anchor::right);
        // Check that anchor fallback is used
        REQUIRE(l2.anchorType() == LabelProperty::Anchor::top);
    }
    labels.clear();

    {
        TextLabel l1 = makeLabelWithAnchorFallbacks(glm::vec2{0.5,0.5});
        l1.update(tile.mvp(), screenSize, true);
        labels.addLabel(&l1, &tile);

        // Second label is 10 pixel below of L1
        TextLabel l2 = makeLabelWithAnchorFallbacks(glm::vec2{0.5,0.5 - 10./256});
        l2.update(tile.mvp(), screenSize, true);
        labels.addLabel(&l2, &tile);

        labels.run(view);
        REQUIRE(l1.isOccluded() == false);
        REQUIRE(l2.isOccluded() == false);

        REQUIRE(l1.anchorType() == LabelProperty::Anchor::right);
        // Check that anchor fallback is used
        REQUIRE(l2.anchorType() == LabelProperty::Anchor::bottom);
    }

}

TEST_CASE( "Regression Test Issue 936", "[Labels]" ) {

    View view(256, 256);
    view.setPosition(0, 0);
    view.setZoom(0);
    view.update(false);

    Tile tile({0,0,0}, view.getMapProjection());
    tile.update(0, view);

    glm::vec2 screenSize = glm::vec2(view.getWidth(), view.getHeight());

    class TestLabels : public Labels {
    public:
        TestLabels(View& _v) {
            m_isect2d.resize({1, 1}, {_v.getWidth(), _v.getHeight()});
        }
        void addLabel(Label* _l, Tile* _t) { m_labels.push_back({_l, _t, false}); }
        void run(View& _v) {
            sortLabels();
            handleOcclusions(_v);
        }
        void clear() { m_labels.clear(); }
    };

    TestLabels labels(view);



    auto parent = makeSpriteLabel(glm::vec2{0.0,1.0}, "A", 0);
    auto child = makeLabel(glm::vec2{0.0,1.0},
                           Label::Type::point, "B", 2, {10, 10},
                           LabelProperty::Anchor::bottom, true);

    float offset = 14.5/256.;
    auto other = makeLabel({glm::vec2{0.0, 1.0 - offset}, glm::vec2{0.0 + offset, 1.0}},
                           Label::Type::line, "C", 1, glm::vec2{14,1});

    child->setParent(*parent, false, false);

    parent->occlude();
    child->occlude();
    other->occlude();

    labels.addLabel(&*parent, &tile);
    labels.addLabel(&*child, &tile);
    labels.addLabel(&*other, &tile);

    {
        parent->update(tile.mvp(), screenSize, true);
        child->update(tile.mvp(), screenSize, true);
        other->update(tile.mvp(), screenSize, true);

        labels.run(view);

        parent->evalState(0);
        child->evalState(0);
        other->evalState(0);

        parent->print();
        child->print();
        other->print();
        LOG("");

        REQUIRE(parent->isOccluded() == false);
        REQUIRE(child->isOccluded() == false);
        REQUIRE(other->isOccluded() == true);
    }

    {
        parent->update(tile.mvp(), screenSize, true);
        child->update(tile.mvp(), screenSize, true);
        other->update(tile.mvp(), screenSize, true);

        labels.run(view);

        parent->evalState(0);
        child->evalState(0);
        other->evalState(0);

        parent->print();
        child->print();
        other->print();
        LOG("");

        REQUIRE(parent->isOccluded() == true);
        REQUIRE(child->isOccluded() == true);
        REQUIRE(other->isOccluded() == false);
    }

    {
        parent->update(tile.mvp(), screenSize, true);
        child->update(tile.mvp(), screenSize, true);
        other->update(tile.mvp(), screenSize, true);

        labels.run(view);

        parent->evalState(0);
        child->evalState(0);
        other->evalState(0);

        parent->print();
        child->print();
        other->print();
        LOG("");

        REQUIRE(parent->isOccluded() == false);
        REQUIRE(child->isOccluded() == false);
        REQUIRE(other->isOccluded() == true);

    }

    {
        parent->update(tile.mvp(), screenSize, true);
        child->update(tile.mvp(), screenSize, true);
        other->update(tile.mvp(), screenSize, true);

        labels.run(view);

        parent->evalState(0);
        child->evalState(0);
        other->evalState(0);

        parent->print();
        child->print();
        other->print();
        LOG("");

        REQUIRE(parent->isOccluded() == true);
        REQUIRE(child->isOccluded() == true);
        REQUIRE(other->isOccluded() == false);

    }


}
}
