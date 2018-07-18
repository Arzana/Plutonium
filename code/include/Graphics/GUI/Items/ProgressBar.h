#pragma once
#include "Graphics\GUI\Core\GuiItem.h"
#include "Graphics\GUI\Core\FillStyle.h"

namespace Plutonium
{
	/* Defines a filling bar to indicate progress. */
	struct ProgressBar
		: public GuiItem
	{
	public:
		/* Occurs when the value of the progress bar changes. */
		EventBus<ProgressBar, ValueChangedEventArgs<float>> ValueChanged;
		/* Occurs when the value of the bar color changes. */
		EventBus<ProgressBar, ValueChangedEventArgs<Color>> BarColorChanged;
		/* Occurs when the value of the bar image is set or changed. */
		EventBus<ProgressBar, ValueChangedEventArgs<TextureHandler>> BarImageChanged;
		/* Occurs when the fill style of the progress bar is changed. */
		EventBus<ProgressBar, ValueChangedEventArgs<FillStyle>> FillStyleChanged;

		/* Initializes a new instance of a progress bar with default parameters. */
		ProgressBar(_In_ Game *parent);
		/* Initializes a new instance of a progress bar. */
		ProgressBar(_In_ Game *parent, _In_ Rectangle bounds);
		ProgressBar(_In_ const ProgressBar &value) = delete;
		ProgressBar(_In_ ProgressBar &&value) = delete;
		/* Releases the resources allocate by the progress bar. */
		~ProgressBar(void);

		_Check_return_ ProgressBar& operator =(_In_ const ProgressBar &other) = delete;
		_Check_return_ ProgressBar& operator =(_In_ ProgressBar &&other) = delete;

		/* Gets the current value of the progress bar mapped to a specified range. */
		_Check_return_ float GetValueMapped(_In_ float min = 0.0f, _In_ float max = 100.0f) const;
		/* Renders the progress bar to the screen. */
		virtual void Draw(_In_ GuiItemRenderer *renderer) override;

		/* Gets the default value for the progress bar bounds. */
		_Check_return_ static inline Rectangle GetDefaultBounds(void)
		{
			return Rectangle(0.0f, 0.0f, 100.0f, 25.0f);
		}

		/* Gets the default value for the bar color. */
		_Check_return_ static inline Color GetDefaultBarColor(void)
		{
			return Color::Lime();
		}

		/* Gets the current used bar filling style. */
		_Check_return_ inline FillStyle GetFillStyle(void) const
		{
			return style;
		}

		/* Gets the current value of the progress bar. */
		_Check_return_ inline float GetValue(void) const
		{
			return value;
		}

		/* Gets the texture used to display the bar (nullptr if none is set). */
		_Check_return_ inline TextureHandler GetBarTexture(void) const
		{
			return bar;
		}

		/* Gets the current color of the bar. */
		_Check_return_ inline Color GetBarColor(void) const
		{
			return barColor;
		}

		/* Sets the bar filling style to the specified value. */
		void SetFillStyle(_In_ FillStyle style);
		/* Set the value with an input that is mapped to a specified range. */
		void SetValueMapped(_In_ float value, _In_ float min = 0.0f, _In_ float max = 100.0f);
		/* Updates the value of the progress bar (0 <= value >= 1). */
		void SetValue(_In_ float value);
		/* Sets the image used to render the bar section of the progress bar. */
		void SetBarImage(_In_ TextureHandler image);
		/* Sets the color (or filter) of the bar section. */
		void SetBarColor(_In_ Color color);

	protected:
		/* Renderes the ProgressBar to the renderer, used for internal item skipping. */
		void RenderProgressBar(_In_ GuiItemRenderer *renderer);

	private:
		FillStyle style;
		float value;
		TextureHandler bar;
		Color barColor;
		Vector2 barPos;
		Buffer *barMesh;

		void OnMoved(const GuiItem*, ValueChangedEventArgs<Vector2> args);
		void UpdateBarMesh(void);
	};
}