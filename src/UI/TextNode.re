module Shaders = Revery_Shaders;
module Geometry = Revery_Geometry;
module Layout = Layout;
module LayoutTypes = Layout.LayoutTypes;

open Fontkit;
open Reglm;
open Revery_Core;

open ViewNode;
open RenderPass;

class textNode (name: string, text: string) = {
    as _this;

    val mutable text = text;
    val quad = Geometry.Cube.create();
    val textureShader = FontShader.create();

    inherit (class viewNode)(name) as _super;
            
    pub! draw = (pass: renderPass, layer: int, world: Mat4.t) => {
        /* Draw background first */
        _super#draw(pass, layer, world);

        switch (pass) {
        | SolidPass(m) => {
            Shaders.CompiledShader.use(textureShader);
    
            Shaders.CompiledShader.setUniformMatrix4fv(textureShader, "uWorld", world);
            Shaders.CompiledShader.setUniformMatrix4fv(textureShader, "uProjection", m);

            let style = _super#getStyle();
            let font = FontCache.load(style.fontFamily, style.fontSize);
            let dimensions = _super#measurements();
            let color = style.color;
            Shaders.CompiledShader.setUniform3fv(textureShader, "uColor", Color.toVec3(color));

            let render = (s: Fontkit.fk_shape, x: float, y: float) => {
                let glyph = FontRenderer.getGlyph(font, s.codepoint);

                let {width, height, bearingX, bearingY, advance, _} = glyph;

                let _ = FontRenderer.getTexture(font, s.codepoint);
                /* TODO: Bind texture */

                Shaders.CompiledShader.setUniform4f(textureShader, "uPosition", 
                    x +. float_of_int(bearingX),
                    y +. float_of_int(dimensions.height) -. float_of_int(bearingY),
                    float_of_int(width),
                    float_of_int(height));

                Geometry.draw(quad, textureShader);

                x +. float_of_int(advance) /. 64.0;
            };

            let shapedText = Fontkit.fk_shape(font, text);
            let startX = ref(float_of_int(dimensions.left));
            Array.iter(s => {
                let nextPosition = render(s, startX^, float_of_int(dimensions.top));
                startX := nextPosition;
            }, shapedText);
            }
        };
    };

    pub setText = (t) => {
        text = t;
    };

    pub! getMeasureFunction = () => {
        let measure = (_mode, _width, _widthMeasureMode, _height, _heightMeasureMode) => {
                /* TODO: Cache font locally in variable */
                let style = _super#getStyle();
                let font = FontCache.load(style.fontFamily, style.fontSize);

                let d = FontRenderer.measure(font, text);
                print_endline ("Measured width: " ++ string_of_int(d.width));
                let ret: Layout.LayoutTypes.dimensions = { LayoutTypes.width: d.width, height: d.height };
                ret;
        };
        Some(measure);
    };
};
