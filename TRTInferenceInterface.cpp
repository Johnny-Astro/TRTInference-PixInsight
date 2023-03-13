#include "TRTInferenceInterface.h"
#include "TRTInferenceParameters.h"
#include "TRTInferenceProcess.h"

#include <pcl/ErrorHandler.h>
#include <pcl/FileDialog.h>
#include <pcl/Settings.h>

namespace pcl
{

TRTInferenceInterface* TheTRTInferenceInterface = nullptr;

TRTInferenceInterface::TRTInferenceInterface()
	: m_instance(TheTRTInferenceProcess)
{
	TheTRTInferenceInterface = this;
}

TRTInferenceInterface::~TRTInferenceInterface()
{
	if (GUI != nullptr)
		delete GUI, GUI = nullptr;
}

IsoString TRTInferenceInterface::Id() const
{
	return TRTInferenceProcess::MODULE_ID;
}

MetaProcess* TRTInferenceInterface::Process() const
{
	return TheTRTInferenceProcess;
}

IsoString TRTInferenceInterface::IconImageSVG() const
{
	return TheTRTInferenceProcess->IconImageSVG();
}

InterfaceFeatures TRTInferenceInterface::Features() const
{
	return InterfaceFeature::Default;
}

void TRTInferenceInterface::ResetInstance()
{
	TRTInferenceInstance defaultInstance(TheTRTInferenceProcess);
	ImportProcess(defaultInstance);
}

bool TRTInferenceInterface::Launch(const MetaProcess& P, const ProcessImplementation*, bool& dynamic, unsigned& /*flags*/)
{
	if (GUI == nullptr)
	{
		GUI = new GUIData(*this);
		SetWindowTitle(TRTInferenceProcess::MODULE_NAME);
		Settings::Read("TRTEngine", m_instance.p_trtEngine);
		UpdateControls();

		// Restore position only
		if (!RestoreGeometry())
			SetDefaultPosition();
		AdjustToContents();
	}

	dynamic = false;
	return &P == TheTRTInferenceProcess;
}

ProcessImplementation* TRTInferenceInterface::NewProcess() const
{
	return new TRTInferenceInstance(m_instance);
}

bool TRTInferenceInterface::ValidateProcess(const ProcessImplementation& p, String& whyNot) const
{
	if (dynamic_cast<const TRTInferenceInstance*>(&p) != nullptr)
		return true;
	whyNot = String("Not a ") + TRTInferenceProcess::MODULE_ID + " instance.";
	return false;
}

bool TRTInferenceInterface::RequiresInstanceValidation() const
{
	return true;
}

bool TRTInferenceInterface::ImportProcess(const ProcessImplementation& p)
{
	m_instance.Assign(p);
	UpdateControls();
	return true;
}

void TRTInferenceInterface::SaveSettings() const
{
	SaveGeometry();
}

void TRTInferenceInterface::UpdateControls()
{
	GUI->TRTEngine_Edit.SetText(m_instance.p_trtEngine);
	Settings::Write("TRTEngine", m_instance.p_trtEngine);
	GUI->TileOverlap_NumericControl.SetValue(m_instance.p_tileOverlap);
	GUI->KeepOutputDimension_CheckBox.SetChecked(m_instance.p_keepOutputDimension);
}

void TRTInferenceInterface::__EditValueUpdated(NumericEdit& sender, double value)
{
	if (sender == GUI->TileOverlap_NumericControl)
		m_instance.p_tileOverlap = value;
}

void TRTInferenceInterface::__Click(Button& sender, bool checked)
{
	if (sender == GUI->TRTEngine_ToolButton)
	{
		OpenFileDialog d;
		d.SetCaption(String(TRTInferenceProcess::MODULE_NAME) + ": Select TensorRT Engine");
		d.AddFilter(FileFilter("TensorRT Engine Files", ".trt"));
		d.AddFilter(FileFilter("Any Files", "*"));
		d.DisableMultipleSelections();
		if (d.Execute())
		{
			m_instance.p_trtEngine = d.FileName();
			UpdateControls();
		}
	}
	else if (sender == GUI->KeepOutputDimension_CheckBox)
	{
		m_instance.p_keepOutputDimension = checked;
	}
}

void TRTInferenceInterface::__EditCompleted(Edit& sender)
{
	try
	{
		String filePath = sender.Text().Trimmed();
		if (sender == GUI->TRTEngine_Edit)
			m_instance.p_trtEngine = filePath;
		UpdateControls();
	}
	ERROR_CLEANUP(
		sender.SelectAll();
	sender.Focus()
		)
}

TRTInferenceInterface::GUIData::GUIData(TRTInferenceInterface& w)
{
	pcl::Font fnt = w.Font();
	int labelWidth1 = fnt.Width(String("TensorRT Engine:") + 'M');
	int editWidth1 = fnt.Width(String('M', 16));

	const char* pythonDllToolTip = "<p>Path to TensorRT Engine.</p>";

	TRTEngine_Label.SetText("TensorRT Engine:");
	TRTEngine_Label.SetFixedWidth(labelWidth1);
	TRTEngine_Label.SetTextAlignment(TextAlign::Right | TextAlign::VertCenter);
	TRTEngine_Label.SetToolTip(pythonDllToolTip);

	TRTEngine_Edit.SetToolTip(pythonDllToolTip);
	TRTEngine_Edit.SetMinWidth(fnt.Width(String('0', 45)));
	TRTEngine_Edit.OnEditCompleted((Edit::edit_event_handler)&TRTInferenceInterface::__EditCompleted, w);

	TRTEngine_ToolButton.SetIcon(w.ScaledResource(":/browser/select-file.png"));
	TRTEngine_ToolButton.SetScaledFixedSize(20, 20);
	TRTEngine_ToolButton.SetToolTip("<p>Select TensorRT Engine</p>");
	TRTEngine_ToolButton.OnClick((Button::click_event_handler)&TRTInferenceInterface::__Click, w);

	TRTEngine_Sizer.SetSpacing(4);
	TRTEngine_Sizer.Add(TRTEngine_Label);
	TRTEngine_Sizer.Add(TRTEngine_Edit, 100);
	TRTEngine_Sizer.Add(TRTEngine_ToolButton);
	TRTEngine_Sizer.AddStretch();

	TRTEngine_Control.SetSizer(TRTEngine_Sizer);

	TileOverlap_NumericControl.label.SetText("Tile Overlap:");
	TileOverlap_NumericControl.label.SetFixedWidth(labelWidth1);
	TileOverlap_NumericControl.slider.SetRange(0, 500);
	TileOverlap_NumericControl.slider.SetScaledMinWidth(300);
	TileOverlap_NumericControl.SetReal();
	TileOverlap_NumericControl.SetRange(TheTRTInferenceTileOverlapParameter->MinimumValue(), TheTRTInferenceTileOverlapParameter->MaximumValue());
	TileOverlap_NumericControl.SetPrecision(TheTRTInferenceTileOverlapParameter->Precision());
	TileOverlap_NumericControl.edit.SetFixedWidth(editWidth1);
	TileOverlap_NumericControl.SetToolTip("<p>The overlap ratio among processing tiles.</p>"
		                                  "<p>The image is processed in tiles, with overlap and feathering among the adjacent tiles to reduce artifacts at tile edges.</p>");
	TileOverlap_NumericControl.OnValueUpdated((NumericEdit::value_event_handler)&TRTInferenceInterface::__EditValueUpdated, w);

	KeepOutputDimension_CheckBox.SetText("Keep Output Dimension");
	KeepOutputDimension_CheckBox.SetToolTip("<p>This is for the AI models with scale-up ratio on the output tensors.</p>"
											"<p>When enabled, the scale-up will be reflected in the result.</p>"
										    "<p>When disabled, resampling will be applied to the output so the result will have the original dimension.</p>");
	KeepOutputDimension_CheckBox.OnClick((Button::click_event_handler)&TRTInferenceInterface::__Click, w);

	Inference_Sizer.SetSpacing(4);
	Inference_Sizer.Add(TileOverlap_NumericControl);
	Inference_Sizer.Add(KeepOutputDimension_CheckBox);

	Inference_Control.SetSizer(Inference_Sizer);

	Global_Sizer.SetMargin(8);
	Global_Sizer.SetSpacing(6);
	Global_Sizer.Add(TRTEngine_Control);
	Global_Sizer.Add(Inference_Control);

	w.SetSizer(Global_Sizer);

	w.EnsureLayoutUpdated();
	w.AdjustToContents();
	w.SetFixedSize();
}

}	// namespace pcl