
## so what is a stickflick
Here's our old code.
```csharp
// Enough capacity for a maximum framerate of 120
private const float _boostFudge = 0.1f;
private const int framerate = 120;
private const float _boostWindow = 0.1f;
private AxesRingBuffer _moveAxesRingBuffer = new((int)((_boostWindow + _boostFudge) * framerate));
private bool _dashBuffered = false;
private float _dashStartTime = 0;
private const float _dashDuration = .43f;
// The punishment dash will actually be stronger than the remaining dashes. This both lets the
// player know that they've missed the chain and gives them more tech to mess with.
private const float _punishmentDashDistance = 250;
private const float _punishmentDashDuration = 1f;
private const float _boostAxesDistanceSqr = 0.9f * 0.9f;
private const float _boostAxesMagnitudeSqr = 0.9f * 0.9f;
// Check for dash. Ensure that dashes can't overlap, as that'd cause disastrous behavior.
if (axes.sqrMagnitude > _boostAxesMagnitudeSqr) {
	for (int index = -1; index > -_moveAxesRingBuffer.Count; --index) {
		var entry = _moveAxesRingBuffer[index];
		if (entry.Time == 0f || time - entry.Time > _boostWindow) {
			break;
		}
		if ((entry.Axes - axes).sqrMagnitude >= _boostAxesDistanceSqr) {
			dashInputThisFrame = true;
			break;
		}
	}
}

public class AxesRingBuffer {
	public class Entry {
		public Vector2 Axes = Vector2.zero;
		public float Time = 0f;
	}

	private Entry[] _items;
	private int _index = 0;

	public int Count => _items.Length;

	public Entry this[int index] {
		get {
			index += _index;
			return _items[(index % Count + Count) % Count];
		}
	}

	public AxesRingBuffer(int count) {
		_items = new Entry[count];
		for (int i = 0; i < count; ++i) {
			_items[i] = new Entry();
		}
	}

	public Entry Next() {
		var result = _items[_index];
		_index = (_index + 1) % Count;
		return result;
	}
}```